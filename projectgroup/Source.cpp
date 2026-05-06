#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <initializer_list>

GLUquadricObj* qobj = NULL;
void drawDetailedDoor(float x, float y, float z, float openAngle);
void drawStudent(float x, float y, float z, float angle, float walkPhase, bool isSeated, int variant, bool isTeaching);
void drawClassroomInterior();

float autoAngle = 0.0f;
float zoom = 15.0f;
float camHeight = 1.2f; // Elevated front view height

// Day/Night & Sky
bool isNight = false;
float cloudOffset = 0.0f;  // clouds drift over time
float sunMoonAngle = 30.0f; // position of sun/moon in the sky
float studentWalkAngle = 0.0f; // students walking on the balcony

// Cinematic State Machine
enum AppMode { MODE_EXTERIOR, MODE_TRANSITION, MODE_CLASSROOM, MODE_TRANSITION_BACK };
AppMode currentMode = MODE_EXTERIOR;

float transitionProgress = 0.0f; // 0.0 to 1.0
float targetDoorAngle = 0.0f; // Center door open angle
float enteringStudentZ = 1.35f; // Student walking in Z pos
float enteringStudentX = 0.0f;  // Student walking in X pos
float carProgress = -40.0f;     // Car animation position

// First-person classroom navigation
float posX = 0.0f, posZ = 0.0f;

//           helpers                                                                                                                                                                                                 
void solidBox(float x1, float y1, float z1, float x2, float y2, float z2,
    float rf, float gf, float bf,   // front colour
    float rs, float gs, float bs,   // side colour
    float rt, float gt, float bt)   // top colour
{
    // front
    glNormal3f(0, 0, 1);
    glColor3f(rf, gf, bf);
    glBegin(GL_QUADS);
    glVertex3f(x1, y1, z2);
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y2, z2);
    glEnd();
    // back
    glNormal3f(0, 0, -1);
    glColor3f(rf * .75f, gf * .75f, bf * .75f);
    glBegin(GL_QUADS);
    glVertex3f(x2, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y2, z1);
    glVertex3f(x2, y2, z1);
    glEnd();
    // left
    glNormal3f(-1, 0, 0);
    glColor3f(rs, gs, bs);
    glBegin(GL_QUADS);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z2);
    glVertex3f(x1, y2, z2);
    glVertex3f(x1, y2, z1);
    glEnd();
    // right
    glNormal3f(1, 0, 0);
    glBegin(GL_QUADS);
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y2, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
    // top
    glNormal3f(0, 1, 0);
    glColor3f(rt, gt, bt);
    glBegin(GL_QUADS);
    glVertex3f(x1, y2, z1);
    glVertex3f(x2, y2, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y2, z2);
    glEnd();
    // bottom
    glNormal3f(0, -1, 0);
    glColor3f(rt * .7f, gt * .7f, bt * .7f);
    glBegin(GL_QUADS);
    glVertex3f(x1, y1, z2);
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y1, z1);
    glVertex3f(x1, y1, z1);
    glEnd();
}

//           environment helpers                                                                                                                                                          
void drawTree(float x, float z, int type)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // Trunk
    glColor3f(0.35f, 0.25f, 0.15f);
    glBegin(GL_QUADS);
    // Simple vertical box for trunk
    float tw = 0.1f;
    glVertex3f(-tw, 0, tw); glVertex3f(tw, 0, tw); glVertex3f(tw, 1.5, tw); glVertex3f(-tw, 1.5, tw);
    glVertex3f(tw, 0, -tw); glVertex3f(-tw, 0, -tw); glVertex3f(-tw, 1.5, -tw); glVertex3f(tw, 1.5, -tw);
    glEnd();

    if (type == 0) { // Pine / Evergreen (Cone shape)
        glColor3f(0.08f, 0.25f, 0.05f);
        glPushMatrix();
        glTranslatef(0, 0.5, 0);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.8, 2.5, 8, 4);
        glPopMatrix();
    }
    else { // Leafy Tree (Sphere-ish)
        glColor3f(0.15f, 0.45f, 0.1f);
        glPushMatrix();
        glTranslatef(0, 2.2, 0);
        glutSolidSphere(1.0, 8, 8);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawCobblestonePath(float x1, float z1, float x2, float z2)
{
    float sw = 0.18f, sd = 0.18f; // Smaller stones
    float gap = 0.04f;
    for (float x = x1; x < x2; x += sw + gap) {
        for (float z = z1; z < z2; z += sd + gap) {
            // Pseudo-random seed based on position
            int seed = (int)((x + 50.0f) * 10.0f) * 131 + (int)((z + 50.0f) * 10.0f);

            // Skip roughly 10% of stones to let grass peek through (organic look)
            if (seed % 11 == 0) continue;

            // Irregularity jitter
            float jx = (float)(seed % 7) * 0.01f;
            float jz = (float)(seed % 9) * 0.01f;

            // Varied grey/stone tones
            float v = (float)(seed % 30) / 100.0f;
            float r = 0.38f + v;
            float g = 0.38f + v;
            float b = 0.40f + v;

            solidBox(x + jx, -0.015f, z + jz, x + sw + jx, 0.015f, z + sd + jz,
                r, g, b, r * 0.8f, g * 0.8f, b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
        }
    }
}

void drawBush(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glColor3f(0.6f, 0.72f, 0.15f); // Bright yellowish green

    // Cluster of 5 spheres for a bushy look
    float offX[5] = { 0.0f, 0.15f, -0.15f, 0.12f, -0.12f };
    float offZ[5] = { 0.0f, 0.12f, 0.12f, -0.15f, -0.15f };
    float offY[5] = { 0.25f, 0.20f, 0.20f, 0.18f, 0.18f };

    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        glTranslatef(offX[i], offY[i], offZ[i]);
        glutSolidSphere(0.25f, 6, 6);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawLongGrass(float x, float z)
{
    glLineWidth(1.0f);
    glColor3f(0.35f, 0.55f, 0.25f);
    glBegin(GL_LINES);
    // Draw 3-4 simple blades of grass
    glVertex3f(x, 0.0f, z); glVertex3f(x - 0.05f, 0.2f, z + 0.05f);
    glVertex3f(x, 0.0f, z); glVertex3f(x + 0.05f, 0.25f, z - 0.02f);
    glVertex3f(x, 0.0f, z); glVertex3f(x, 0.18f, z + 0.08f);
    glEnd();
}

void drawDitch(float xCenter, float z1, float z2)
{
    float w = 0.5f; // ditch width
    float d = 0.25f; // depth
    glColor3f(0.42f, 0.48f, 0.38f); // Mossy concrete grey-green
    glBegin(GL_QUADS);
    // Left slope
    glNormal3f(0.447f, 0.894f, 0.0f); // Approximate normal for slanted face
    glVertex3f(xCenter - w / 2, 0.0f, z1); glVertex3f(xCenter, -d, z1);
    glVertex3f(xCenter, -d, z2); glVertex3f(xCenter - w / 2, 0.0f, z2);
    // Right slope
    glNormal3f(-0.447f, 0.894f, 0.0f);
    glVertex3f(xCenter, -d, z1); glVertex3f(xCenter + w / 2, 0.0f, z1);
    glVertex3f(xCenter + w / 2, 0.0f, z2); glVertex3f(xCenter, -d, z2);
    glEnd();
}

void drawDitchHorizontal(float zCenter, float x1, float x2)
{
    float w = 0.5f; // ditch width
    float d = 0.25f; // depth
    glColor3f(0.42f, 0.48f, 0.38f); // Mossy concrete grey-green
    glBegin(GL_QUADS);
    // Back slope
    glNormal3f(0.0f, 0.894f, 0.447f);
    glVertex3f(x1, 0.0f, zCenter - w / 2); glVertex3f(x1, -d, zCenter);
    glVertex3f(x2, -d, zCenter); glVertex3f(x2, 0.0f, zCenter - w / 2);
    // Front slope
    glNormal3f(0.0f, 0.894f, -0.447f);
    glVertex3f(x1, -d, zCenter); glVertex3f(x1, 0.0f, zCenter + w / 2);
    glVertex3f(x2, 0.0f, zCenter + w / 2); glVertex3f(x2, -d, zCenter);
    glEnd();
}

void drawDetailedDoor(float x, float y, float z, float openAngle = 0.0f)
{
    float w = 0.52f;
    float h = 0.7f; // Main door height
    float transomH = 0.2f; // Window above door

    // 1. Transom Window (Above door)
    glColor3f(0.05f, 0.05f, 0.12f); // Dark glass
    glBegin(GL_QUADS);
    glVertex3f(x - w / 2, y + h, z); glVertex3f(x + w / 2, y + h, z);
    glVertex3f(x + w / 2, y + h + transomH, z); glVertex3f(x - w / 2, y + h + transomH, z);
    glEnd();

    // 6. Frame
    glColor3f(0.75f, 0.72f, 0.68f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x - w / 2, y, z + 0.001f); glVertex3f(x + w / 2, y, z + 0.001f);
    glVertex3f(x + w / 2, y + h + transomH, z + 0.001f); glVertex3f(x - w / 2, y + h + transomH, z + 0.001f);
    glEnd();

    // 0. Dark Interior Hole (Revealed when door opens)
    // Using a thin box instead of a quad to ensure it masks the wall and has depth
    solidBox(x - w / 2, y, z - 0.01f, x + w / 2, y + h, z - 0.001f, 
             0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    glPushMatrix();
    // Hinge on the left side
    glTranslatef(x - w / 2, y, z);
    glRotatef(openAngle, 0, 1, 0);
    glTranslatef(-(x - w / 2), -y, -z);

    // 2. Door Surface (Light Tan)
    glColor3f(0.92f, 0.88f, 0.82f);
    glBegin(GL_QUADS);
    glVertex3f(x - w / 2, y, z); glVertex3f(x + w / 2, y, z);
    glVertex3f(x + w / 2, y + h, z); glVertex3f(x - w / 2, y + h, z);
    glEnd();

    // 3. Vertical Ridges (Thin lines)
    glColor3f(0.75f, 0.72f, 0.68f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float rx = -w / 2 + 0.06f; rx <= w / 2 - 0.06f; rx += 0.06f) {
        glVertex3f(x + rx, y, z + 0.005f); glVertex3f(x + rx, y + h, z + 0.005f);
    }
    glEnd();

    // 4. Horizontal Middle Bar
    glColor3f(0.88f, 0.84f, 0.80f);
    glBegin(GL_QUADS);
    glVertex3f(x - w / 2, y + h / 2 - 0.02f, z + 0.008f); glVertex3f(x + w / 2, y + h / 2 - 0.02f, z + 0.008f);
    glVertex3f(x + w / 2, y + h / 2 + 0.02f, z + 0.008f); glVertex3f(x - w / 2, y + h / 2 + 0.02f, z + 0.008f);
    glEnd();

    // 5. Handle/Lock Detail
    glColor3f(0.35f, 0.35f, 0.35f);
    glPushMatrix();
    glTranslatef(x + w / 2 - 0.1f, y + h / 2, z + 0.015f);
    glutSolidCube(0.04f);
    glPopMatrix();

    glPopMatrix(); // End of door hinge
}

void drawStudent(float x, float y, float z, float angle, float walkPhase = 0.0f, bool isSeated = false, int variant = 0, bool isTeaching = false)
{
    if (!qobj) {
        qobj = gluNewQuadric();
        gluQuadricNormals(qobj, GLU_SMOOTH);
    }

    glPushMatrix();
    
    // Teaching movement (ultra slow pacing and turning)
    float tx = x;
    float teacherAngle = angle;
    if (isTeaching) {
        tx += sinf(walkPhase * 0.05f) * 0.6f;
        // Turn to face the whiteboard periodically (when sin is high)
        if (sinf(walkPhase * 0.1f) > 0.6f) teacherAngle += 180.0f;
    }

    // Body bobbing
    float bob = (isSeated || isTeaching) ? 0.0f : fabsf(sinf(walkPhase * 2.0f)) * 0.05f;
    glTranslatef(tx, y + bob, z);
    glRotatef(teacherAngle, 0, 1, 0);

    // Teachers are slightly taller (15% more)
    float scale = isTeaching ? 0.52f : 0.45f;
    glScalef(scale, scale, scale);

    // 1. COLORS
    float shirtColors[6][3] = {
        {0.2f, 0.3f, 0.8f}, {0.8f, 0.2f, 0.2f}, {0.2f, 0.7f, 0.3f},
        {0.9f, 0.8f, 0.2f}, {0.6f, 0.2f, 0.7f}, {0.2f, 0.8f, 0.8f}
    };
    float teacherSuit[3] = {0.15f, 0.15f, 0.2f}; // Dark formal suit
    
    float hairColors[4][3] = {
        {0.05f, 0.05f, 0.05f}, {0.35f, 0.2f, 0.1f}, {0.8f, 0.7f, 0.3f}, {0.5f, 0.5f, 0.5f}
    };
    int sCol = variant % 6;
    int hCol = variant % 4;

    // 2. TORSO
    if (isTeaching) glColor3fv(teacherSuit);
    else glColor3fv(shirtColors[sCol]);

    solidBox(-0.22f, 0.45f, -0.12f, 0.22f, 1.05f, 0.12f, 
             isTeaching ? 0.15f : shirtColors[sCol][0], isTeaching ? 0.15f : shirtColors[sCol][1], isTeaching ? 0.2f : shirtColors[sCol][2],
             0.1f, 0.1f, 0.12f, 0.2f, 0.2f, 0.25f);

    // 2.5 PANTS / PELVIS
    glColor3f(0.1f, 0.12f, 0.15f);
    solidBox(-0.22f, 0.35f, -0.12f, 0.22f, 0.45f, 0.12f, 0.1f, 0.12f, 0.15f, 0.05f, 0.06f, 0.08f, 0.15f, 0.18f, 0.2f);

    // 3. HEAD
    glPushMatrix();
    glTranslatef(0.0f, 1.25f, 0.0f);
    glColor3f(0.9f, 0.7f, 0.6f);
    glutSolidSphere(0.22f, 20, 16);
    
    glPushMatrix(); // Hair
    glTranslatef(0.0f, 0.08f, 0.0f);
    glScalef(1.05f, 0.7f, 1.05f);
    glColor3fv(hairColors[hCol]);
    glutSolidSphere(0.22f, 16, 12);
    glPopMatrix();

    glColor3f(1, 1, 1); // Eyes
    float eyeX[] = {-0.08f, 0.08f};
    for (int i = 0; i < 2; i++) {
        float ex = eyeX[i];
        glPushMatrix();
        glTranslatef(ex, 0.02f, 0.18f);
        glutSolidSphere(0.04f, 10, 10);
        glColor3f(0, 0, 0);
        glTranslatef(0, 0, 0.03f);
        glutSolidSphere(0.02f, 8, 8);
        glColor3f(1, 1, 1);
        glPopMatrix();
    }
    glPopMatrix();

    // 4. NECK
    glColor3f(0.85f, 0.65f, 0.55f);
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(qobj, 0.06, 0.06, 0.1, 16, 1);
    glPopMatrix();

    // 5. LEGS
    float legSwing = (isSeated || isTeaching) ? 0.0f : sinf(walkPhase) * 35.0f;
    float legX[] = {-0.12f, 0.12f};
    for (int i = 0; i < 2; i++) {
        float lx = legX[i];
        glPushMatrix();
        glTranslatef(lx, 0.40f, 0.0f);
        if (isSeated) {
            glRotatef(-90, 1, 0, 0);
            glRotatef((lx < 0 ? -1 : 1) * 5, 0, 1, 0);
        } else {
            glRotatef(lx < 0 ? legSwing : -legSwing, 1, 0, 0);
        }
        
        glColor3f(0.1f, 0.12f, 0.15f);
        glRotatef(90, 1, 0, 0);
        gluCylinder(qobj, 0.08, 0.06, 0.4, 16, 2);
        
        glTranslatef(0, 0, 0.4f); // Knee
        glutSolidSphere(0.06f, 10, 10);
        
        if (isSeated) glRotatef(90, 1, 0, 0);
        glColor3f(0.1f, 0.12f, 0.15f);
        gluCylinder(qobj, 0.06, 0.04, 0.4, 16, 2);
        glPopMatrix();
    }

    // 6. ARMS
    float armSwing = (isSeated || isTeaching) ? 0.0f : sinf(walkPhase) * 45.0f;
    float armX[] = {-0.25f, 0.25f};
    for (int i = 0; i < 2; i++) {
        float ax = armX[i];
        glPushMatrix();
        glTranslatef(ax, 1.0f, 0.0f);
        
        if (isSeated) {
            glRotatef(-30, 1, 0, 0);
            glRotatef(ax < 0 ? 15 : -15, 0, 0, 1);
        } else if (isTeaching) {
            // Both arms in active lecturing/pointing pose
            glRotatef(-50, 1, 0, 0); 
            glRotatef(ax < 0 ? 20 : -20, 0, 1, 0); // Angled inward
        } else {
            glRotatef(ax < 0 ? -armSwing : armSwing, 1, 0, 0);
        }
        
        if (isTeaching) glColor3fv(teacherSuit);
        else glColor3fv(shirtColors[sCol]);
        
        glRotatef(90, 1, 0, 0);
        gluCylinder(qobj, 0.06, 0.05, 0.35, 16, 2);
        
        glTranslatef(0, 0, 0.35f); // Elbow
        glutSolidSphere(0.05f, 10, 10);
        
        glColor3f(0.9f, 0.7f, 0.6f);
        if (isSeated) glRotatef(-60, 1, 0, 0);
        else if (isTeaching) glRotatef(-40, 1, 0, 0); // Professional bend
        
        gluCylinder(qobj, 0.05, 0.04, 0.35, 16, 2);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawStudents()
{
    // Draw students walking back and forth near the first floor doors (balcony)
    for (int i = 0; i < 3; i++) {
        float phaseOffset = i * 120.0f; // phase offset
        float studentRad1 = (studentWalkAngle + phaseOffset) * 3.14159f / 180.0f;
        
        // Elongated oval path on the balcony
        // Balcony X is roughly -4.0 to 4.0, Z is 0.7 to 1.65
        float studentPosX1 = sinf(studentRad1) * 3.5f;
        float studentPosZ1 = 1.15f + cosf(studentRad1) * 0.15f; 
        float studentPosY1 = 1.08f; // First floor balcony floor height
        
        // Calculate tangent for smooth turning at the ends
        float studentDirX1 = cosf(studentRad1) * 3.5f;
        float studentDirZ1 = -sinf(studentRad1) * 0.15f;
        float studentFacing1 = atan2f(studentDirX1, studentDirZ1) * 180.0f / 3.14159f;
        
        drawStudent(studentPosX1, studentPosY1, studentPosZ1, studentFacing1, (studentWalkAngle + phaseOffset) * 0.1f, false, i);

        // Ground floor students pacing near the ground floor doors
        // If we are transitioning, student 0 detaches and becomes the entering student
        if (currentMode != MODE_EXTERIOR && i == 0) {
            drawStudent(enteringStudentX, 0.0f, enteringStudentZ, 180.0f, transitionProgress * 20.0f, false, i + 10); // facing inside
        } else {
            // Give them a slightly different speed multiplier and phase
            float studentRad2 = (studentWalkAngle * 0.8f + phaseOffset + 60.0f) * 3.14159f / 180.0f;
            float studentPosX2 = sinf(studentRad2) * 3.5f;
            float studentPosZ2 = 1.35f + cosf(studentRad2) * 0.15f; // Ground floor walkway (between 0.7 and 1.8)
            float studentPosY2 = 0.0f; // Ground floor height
            
            float studentDirX2 = cosf(studentRad2) * 3.5f;
            float studentDirZ2 = -sinf(studentRad2) * 0.15f;
            float studentFacing2 = atan2f(studentDirX2, studentDirZ2) * 180.0f / 3.14159f;

            drawStudent(studentPosX2, studentPosY2, studentPosZ2, studentFacing2, (studentWalkAngle * 0.8f + phaseOffset + 60.0f) * 0.1f, false, i + 20);
        }
    }
}

void drawClassroomInterior()
{
    // Draw the hollowed-out room on the ground floor behind the middle door.
    // Expanded for ~41 students (6 columns x 7 rows = 42 students)
    float rX1 = -3.5f, rX2 = 3.5f;
    float rZ1 = -7.5f, rZ2 = 0.65f; 
    float rY1 = 0.01f, rY2 = 2.5f; // Taller room

    // Floor (Light Tile pattern)
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(rX1, rY1, rZ2); glVertex3f(rX2, rY1, rZ2);
    glVertex3f(rX2, rY1, rZ1); glVertex3f(rX1, rY1, rZ1);
    glEnd();

    // Ceiling (Light)
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(rX1, rY2, rZ1); glVertex3f(rX2, rY2, rZ1);
    glVertex3f(rX2, rY2, rZ2); glVertex3f(rX1, rY2, rZ2);
    glEnd();

    // Walls - Universal two-tone logic
    float walls[][6] = {
        {rX1, rY1, rZ1, rX1, rY2, rZ2}, // Left
        {rX2, rY1, rZ2, rX2, rY2, rZ1}, // Right
        {rX1, rY1, rZ1, rX2, rY2, rZ1}, // Front
        {rX2, rY1, rZ2, rX1, rY2, rZ2}  // Back
    };
    float normals[][3] = {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}};

    for (int i = 0; i < 4; i++) {
        glBegin(GL_QUADS);
        glNormal3fv(normals[i]);
        // Bottom Grey Band
        glColor3f(0.35f, 0.38f, 0.41f);
        glVertex3f(walls[i][0], walls[i][1], walls[i][2]);
        glVertex3f(walls[i][3], walls[i][1], walls[i][5]);
        glVertex3f(walls[i][3], 1.2f, walls[i][5]);
        glVertex3f(walls[i][0], 1.2f, walls[i][2]);
        // Top White Band
        glColor3f(0.92f, 0.92f, 0.92f);
        glVertex3f(walls[i][0], 1.2f, walls[i][2]);
        glVertex3f(walls[i][3], 1.2f, walls[i][5]);
        glVertex3f(walls[i][3], walls[i][4], walls[i][5]);
        glVertex3f(walls[i][0], walls[i][4], walls[i][2]);
        glEnd();
    }

    // Door at the back (Shifted to right and using exterior style)
    glPushMatrix();
    glTranslatef(2.5f, 0.01f, rZ2 - 0.01f);
    glRotatef(180, 0, 1, 0); // Face inside
    drawDetailedDoor(0.0f, 0.0f, 0.0f, 0.0f); 
    glPopMatrix();

    // Whiteboard
    solidBox(-1.2f, 0.6f, rZ1 + 0.01f, 0.2f, 1.6f, rZ1 + 0.05f, 0.95f, 0.95f, 0.95f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f);
    
    // TV
    solidBox(0.6f, 0.8f, rZ1 + 0.01f, 1.8f, 1.6f, rZ1 + 0.06f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f);
    // TV Screen (glow)
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.4f, 0.8f);
    glBegin(GL_QUADS);
    glVertex3f(0.62f, 0.82f, rZ1 + 0.061f); glVertex3f(1.78f, 0.82f, rZ1 + 0.061f);
    glVertex3f(1.78f, 1.58f, rZ1 + 0.061f); glVertex3f(0.62f, 1.58f, rZ1 + 0.061f);
    glEnd();
    glEnable(GL_LIGHTING);

    // PROJECTOR (Hanging from ceiling)
    float pX = -0.5f; 
    float pY = rY2 - 0.2f; 
    float pZ = -4.0f; 
    
    // Mount pole
    solidBox(pX - 0.02f, pY, pZ - 0.02f, pX + 0.02f, rY2, pZ + 0.02f, 0.2f, 0.2f, 0.2f, 0.1f, 0.1f, 0.1f, 0.3f, 0.3f, 0.3f);
    // Projector Body
    solidBox(pX - 0.15f, pY - 0.15f, pZ - 0.2f, pX + 0.15f, pY, pZ + 0.2f, 0.9f, 0.9f, 0.9f, 0.7f, 0.7f, 0.7f, 1.0f, 1.0f, 1.0f);
    // Lens
    glPushMatrix();
    glTranslatef(pX, pY - 0.08f, pZ - 0.2f);
    glColor3f(0.1f, 0.1f, 0.1f);
    gluCylinder(qobj, 0.05, 0.05, 0.05, 12, 1);
    // Lens glass
    glTranslatef(0, 0, -0.01f);
    glColor3f(0.4f, 0.6f, 1.0f);
    gluDisk(qobj, 0, 0.04, 12, 1);
    glPopMatrix();

    // Teacher (Standing and Teaching)
    drawStudent(-0.5f, 0.0f, rZ1 + 0.8f, 0.0f, studentWalkAngle, false, 0, true); 
    
    // Desks, Chairs and Students (6 columns x 7 rows = 42 students)
    int studentIdx = 0;
    for (float dx = -2.5f; dx <= 2.5f; dx += 1.0f) {
        for (float dz = -5.5f; dz <= 0.0f; dz += 0.9f) {
            // Desk
            solidBox(dx - 0.35f, 0.0f, dz - 0.2f, dx + 0.35f, 0.35f, dz + 0.1f, 0.6f, 0.4f, 0.2f, 0.5f, 0.3f, 0.15f, 0.65f, 0.45f, 0.25f);
            
            // Chair (Seat)
            solidBox(dx - 0.15f, 0.0f, dz + 0.15f, dx + 0.15f, 0.15f, dz + 0.4f, 0.3f, 0.2f, 0.1f, 0.25f, 0.15f, 0.05f, 0.35f, 0.25f, 0.15f);
            // Chair (Backrest)
            solidBox(dx - 0.15f, 0.15f, dz + 0.35f, dx + 0.15f, 0.45f, dz + 0.4f, 0.3f, 0.2f, 0.1f, 0.25f, 0.15f, 0.05f, 0.35f, 0.25f, 0.15f);

            // Seated Student
            drawStudent(dx, -0.1f, dz + 0.2f, 180.0f, 0.0f, true, studentIdx++); 
        }
    }
}


//           sky helpers

void drawStar(float x, float y, float z)
{
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glVertex3f(x, y, z);
    glEnd();
}

void drawStars()
{
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.95f);
    // Fixed star field using deterministic positions
    for (int i = 0; i < 200; i++) {
        float seed = (float)(i * 1973 + 42);
        float px = fmodf(seed * 0.0137f, 160.0f) - 80.0f;
        float py = 15.0f + fmodf(seed * 0.0271f, 25.0f);
        float pz = fmodf(seed * 0.0193f, 160.0f) - 80.0f;
        // Vary brightness
        float bright = 0.6f + fmodf(seed * 0.041f, 0.4f);
        glColor3f(bright, bright, bright * 0.95f);
        drawStar(px, py, pz);
    }
    glEnable(GL_LIGHTING);
}

void drawMoon(float angle)
{
    glDisable(GL_LIGHTING);
    glPushMatrix();
    // Position moon in sky
    glRotatef(angle, 0, 0, 1);
    glTranslatef(0.0f, 30.0f, -60.0f);

    // Moon body (pale yellow-white)
    glColor3f(0.95f, 0.95f, 0.80f);
    glutSolidSphere(2.5f, 16, 16);

    // Crescent shadow (dark overlay slightly offset)
    glColor3f(0.12f, 0.16f, 0.28f);
    glTranslatef(0.8f, 0.3f, 0.3f);
    glutSolidSphere(2.1f, 16, 16);

    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawSun(float angle)
{
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glRotatef(angle, 0, 0, 1);
    glTranslatef(0.0f, 30.0f, -60.0f);

    // --- Rays (drawn first, behind the sun body) ---
    int numRays = 16;
    float innerR = 3.2f;   // where ray starts (just outside core)
    float outerR = 8.5f;   // where long rays end
    float shortR = 6.0f;   // where short rays end
    float rayHalfW = 0.18f; // half-width of each ray at the base

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numRays; i++) {
        float a = (float)i / numRays * 3.14159f * 2.0f;
        float a1 = a - rayHalfW / innerR;
        float a2 = a + rayHalfW / innerR;

        // Alternate long and short rays
        float rOuter = (i % 2 == 0) ? outerR : shortR;

        // Bright yellow at base, fades to transparent-ish orange at tip
        glColor4f(1.0f, 0.95f, 0.3f, 0.9f);
        glVertex3f(cosf(a1) * innerR, sinf(a1) * innerR, 0.0f);
        glVertex3f(cosf(a2) * innerR, sinf(a2) * innerR, 0.0f);

        glColor4f(1.0f, 0.75f, 0.1f, 0.0f); // fade out at tip
        glVertex3f(cosf(a) * rOuter, sinf(a) * rOuter, 0.0f);
    }
    glEnd();

    // --- Outer soft glow halo ---
    glColor3f(1.0f, 0.90f, 0.35f);
    glutSolidSphere(3.5f, 20, 20);

    // --- Bright core ---
    glColor3f(1.0f, 1.0f, 0.85f);
    glutSolidSphere(2.5f, 20, 20);

    // --- Hot white center ---
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidSphere(1.4f, 16, 16);

    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawCloud(float x, float y, float z, float scale)
{
    glDisable(GL_LIGHTING);
    if (isNight)
        glColor3f(0.25f, 0.28f, 0.35f); // Dark blue-grey at night
    else
        glColor3f(1.0f, 1.0f, 1.0f);    // Bright white in day

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale * 0.6f, scale);

    // Cluster of spheres for fluffy cloud
    float offX[] = { 0.0f,  1.2f, -1.2f,  0.7f, -0.7f,  2.0f, -2.0f };
    float offY[] = { 0.5f,  0.2f,  0.2f,  0.8f,  0.8f,  0.0f,  0.0f };
    float offZ[] = { 0.0f,  0.3f,  0.3f, -0.2f, -0.2f,  0.5f,  0.5f };
    float rad[] = { 1.2f,  1.0f,  1.0f,  0.9f,  0.9f,  0.8f,  0.8f };

    for (int i = 0; i < 7; i++) {
        glPushMatrix();
        glTranslatef(offX[i], offY[i], offZ[i]);
        glutSolidSphere(rad[i], 8, 6);
        glPopMatrix();
    }
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawSky()
{
    glDisable(GL_LIGHTING);

    // Stars (night only)
    if (isNight) drawStars();

    // Sun or Moon
    if (isNight)
        drawMoon(sunMoonAngle);
    else
        drawSun(sunMoonAngle);

    // Clouds — 5 clouds drifting across the sky
    float baseZ = -50.0f;
    drawCloud(-30.0f + cloudOffset, 18.0f, baseZ, 1.8f);
    drawCloud(-5.0f + cloudOffset, 22.0f, baseZ - 10.f, 1.4f);
    drawCloud(20.0f + cloudOffset, 19.0f, baseZ + 5.f, 2.0f);
    drawCloud(-50.0f + cloudOffset, 20.0f, baseZ - 5.f, 1.6f);
    drawCloud(45.0f + cloudOffset, 21.0f, baseZ, 1.5f);

    glEnable(GL_LIGHTING);
}

void drawCar(float x, float y, float z, float angle)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angle, 0, 1, 0);

    // 1. Lower Body (Black)
    glColor3f(0.05f, 0.05f, 0.05f);
    solidBox(-0.6f, 0.1f, -1.2f, 0.6f, 0.45f, 1.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f, 0.05f, 0.12f, 0.12f, 0.12f);

    // 2. Upper Cabin / Roof
    glColor3f(0.08f, 0.08f, 0.08f);
    solidBox(-0.55f, 0.45f, -0.5f, 0.55f, 0.85f, 0.6f, 0.15f, 0.15f, 0.15f, 0.1f, 0.1f, 0.1f, 0.2f, 0.2f, 0.2f);

    // 3. Windows (Dark Blue-Grey tint)
    glColor3f(0.15f, 0.2f, 0.3f);
    // Front Windshield
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0.45f, 0.61f); glVertex3f(0.5f, 0.45f, 0.61f);
    glVertex3f(0.45f, 0.78f, 0.52f); glVertex3f(-0.45f, 0.78f, 0.52f);
    // Back Window
    glVertex3f(-0.5f, 0.45f, -0.51f); glVertex3f(0.5f, 0.45f, -0.51f);
    glVertex3f(0.45f, 0.78f, -0.42f); glVertex3f(-0.45f, 0.78f, -0.42f);
    glEnd();

    // 4. Wheels (Dark Grey with silver rims)
    glColor3f(0.1f, 0.1f, 0.1f);
    float wheelX[] = {-0.6f, 0.6f, -0.6f, 0.6f};
    float wheelZ[] = {0.75f, 0.75f, -0.75f, -0.75f};
    for(int i=0; i<4; i++) {
        glPushMatrix();
        glTranslatef(wheelX[i], 0.18f, wheelZ[i]);
        glRotatef(90, 0, 1, 0);
        if(i%2 == 0) glRotatef(180, 0, 1, 0);
        // Tire
        glColor3f(0.15f, 0.15f, 0.15f);
        gluCylinder(qobj, 0.18, 0.18, 0.15, 16, 2);
        // Rim
        glTranslatef(0,0,0.151f);
        glColor3f(0.6f, 0.6f, 0.65f);
        gluDisk(qobj, 0, 0.12, 16, 2);
        glPopMatrix();
    }

    // 5. Headlights & Tail lights
    // Front (White/Yellow)
    glColor3f(1.0f, 1.0f, 0.8f);
    solidBox(-0.55f, 0.25f, 1.2f, -0.35f, 0.35f, 1.22f, 1.0f, 1.0f, 0.8f, 0.9f, 0.9f, 0.7f, 1.0f, 1.0f, 0.9f);
    solidBox(0.35f, 0.25f, 1.2f, 0.55f, 0.35f, 1.22f, 1.0f, 1.0f, 0.8f, 0.9f, 0.9f, 0.7f, 1.0f, 1.0f, 0.9f);
    // Back (Red)
    glColor3f(0.8f, 0.0f, 0.0f);
    solidBox(-0.55f, 0.25f, -1.22f, -0.35f, 0.35f, -1.2f, 0.8f, 0.0f, 0.0f, 0.7f, 0.0f, 0.0f, 0.9f, 0.1f, 0.1f);
    solidBox(0.35f, 0.25f, -1.22f, 0.55f, 0.35f, -1.2f, 0.8f, 0.0f, 0.0f, 0.7f, 0.0f, 0.0f, 0.9f, 0.1f, 0.1f);

    glPopMatrix();
}

//           earth                                                                                                                                                                                                          
void drawEarth()
{
    // 1. Lush Grass Ground (Restored)
    glColor3f(0.25f, 0.45f, 0.15f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-100.f, -0.02f, 100.f); glVertex3f(100.f, -0.02f, 100.f);
    glVertex3f(100.f, -0.02f, -100.f); glVertex3f(-100.f, -0.02f, -100.f);
    glEnd();

    // 1.5 Asphalt Road (Refined lane)
    glColor3f(0.22f, 0.22f, 0.25f); 
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-100.f, -0.015f, 12.0f); glVertex3f(100.f, -0.015f, 12.0f);
    glVertex3f(100.f, -0.015f, 17.0f); glVertex3f(-100.f, -0.015f, 17.0f);
    glEnd();

    // Road Markings (Dashed white line)
    glColor3f(0.95f, 0.95f, 0.95f);
    for (float x = -98.0f; x <= 98.0f; x += 6.0f) {
        glBegin(GL_QUADS);
        glVertex3f(x, -0.014f, 14.4f); glVertex3f(x + 3.0f, -0.014f, 14.4f);
        glVertex3f(x + 3.0f, -0.014f, 14.6f); glVertex3f(x, -0.014f, 14.6f);
        glEnd();
    }

    // 2. Concrete Walkways & Drainage Ditches
    // Side Cobblestone Walkways (Pushed outward from the building)
    drawCobblestonePath(-10.75f, -15.0f, -7.75f, 7.5f);
    drawCobblestonePath(7.75f, -15.0f, 10.75f, 7.5f);

    // NEW: Front Drainage Ditch (Building -> Ditch -> Hedge layout)
    drawDitchHorizontal(2.1f, -6.0f, 6.0f);

    // Solid gray ground strip in front of plinth
    solidBox(-5.8f, -0.01f, 1.8f, 5.8f, 0.01f, 2.5f, 0.45f, 0.45f, 0.45f, 0.4f, 0.4f, 0.4f, 0.5f, 0.5f, 0.5f);

    // 6. Landscaping (Trees & Bushes)
    // Rectangular hedge matching the red line layout
    // Back edge (closest to building, Z = 3.5)
    for (float x = -6.0f; x <= 4.2f; x += 0.60f)
        drawBush(x, 3.5f);
    // Right edge (x = 4.2, from z=3.5 to z=6.5)
    for (float z = 4.1f; z <= 6.5f; z += 0.6f)
        drawBush(4.2f, z);
    // Front/bottom edge (z = 6.5, from x=4.2 to x=-6.0)
    for (float x = -5.4f; x <= 4.2f; x += 0.6f)
        drawBush(x, 6.5f);
    // Left edge (x = -6.0, from z=3.5 to z=6.5)
    for (float z = 4.1f; z <= 6.5f; z += 0.6f)
        drawBush(-6.0f, z);

    // Dense Long Grass inside the rectangular hedge
    for (float x = -5.5f; x <= 3.7f; x += 0.5f) {
        for (float z = 4.0f; z <= 7.0f; z += 0.5f) {
            // Jitter for organic look
            float jx = (float)(rand() % 10) / 20.0f - 0.25f;
            float jz = (float)(rand() % 10) / 20.0f - 0.25f;
            drawLongGrass(x + jx, z + jz);
        }
    }

    // Avenue of Trees along the Left Cobblestone Path (Pushed outwards)
    for (float z = -14.3f; z <= 0.8f; z += 3.0f) {
        drawTree(-11.75f, z, 1); // Outer side
        drawTree(-6.75f, z, 1);  // Inner side
    }

    // Avenue of Trees along the Right Cobblestone Path (Pushed outwards)
    for (float z = -14.3f; z <= 0.8f; z += 3.0f) {
        drawTree(11.75f, z, 1); // Outer side
        drawTree(6.75f, z, 1);  // Inner side
    }

    // Background forest line
    for (float x = -40.0f; x <= 40.0f; x += 10.0f) {
        drawTree(x, -12.0f, 1);
    }
}

//           stairs (U-shaped / Dog-leg)                                                                                                                                        
void drawStaircase(float xOuter, float xInner, float zFront, float zBack)
{
    float midX = (xOuter + xInner) * 0.5f;
    float yMid = 0.51f;
    float yTop = 1.00f;
    int   N = 6;
    float gap = 0.12f;
    float midX_Inner = midX + (xInner > xOuter ? gap : -gap);
    float midX_Outer = midX + (xOuter > xInner ? gap : -gap);
    float lDepth = 0.55f;
    float density = 0.06f;
    float rSize = 0.015f;
    float rH = 0.45f;  // railing post height
    bool  leftSide = (xOuter < xInner);  // true = left staircase

    // sign helpers: +1 or -1 for outward/inward direction
    float outSign = leftSide ? -1.0f : 1.0f; // outward from centre
    float inSign = leftSide ? 1.0f : -1.0f;

    //  ── STEP GEOMETRY ───────────────────────────────────────────────
    // Flight 1: ground → mid landing
    for (int i = 0; i < N; i++) {
        float f = (float)i / N;
        float yb = f * yMid;
        float yt = (float)(i + 1) / N * yMid;
        float zf = zFront - f * (zFront - zBack);
        float zb = zFront - (float)(i + 1) / N * (zFront - zBack);
        solidBox(midX_Inner, yt - 0.02f, zb, xInner, yt, zf, 0.60f, 0.60f, 0.60f, 0.50f, 0.50f, 0.50f, 0.65f, 0.65f, 0.65f);
        solidBox(midX_Inner, yb, zb - 0.02f, xInner, yt - 0.02f, zb, 0.50f, 0.50f, 0.50f, 0.40f, 0.40f, 0.40f, 0.45f, 0.45f, 0.45f);
    }

    // Mid landing platform
    solidBox(xOuter, yMid - 0.02f, zBack - lDepth, xInner, yMid, zBack,
        0.60f, 0.60f, 0.60f, 0.50f, 0.50f, 0.50f, 0.65f, 0.65f, 0.65f);

    // Flight 2: mid landing → top balcony
    for (int i = 0; i < N; i++) {
        float f = (float)i / N;
        float yb = yMid + f * (yTop - yMid);
        float yt = yMid + (float)(i + 1) / N * (yTop - yMid);
        float zb = zBack + f * (zFront - zBack);
        float zf = zBack + (float)(i + 1) / N * (zFront - zBack);
        solidBox(xOuter, yt - 0.02f, zb, midX_Outer, yt, zf, 0.60f, 0.60f, 0.60f, 0.50f, 0.50f, 0.50f, 0.65f, 0.65f, 0.65f);
        solidBox(xOuter, yb, zb - 0.02f, midX_Outer, yt - 0.02f, zb, 0.50f, 0.50f, 0.50f, 0.40f, 0.40f, 0.40f, 0.45f, 0.45f, 0.45f);
    }

    // Top landing platform
    solidBox(xOuter, yTop - 0.08f, 0.7f, xInner, yTop, 1.65f,
        0.72f, 0.72f, 0.72f, 0.58f, 0.58f, 0.58f, 0.66f, 0.66f, 0.66f);

    //  ── RAILING POSTS ───────────────────────────────────────────────
    // Helper lambda: draw a single baluster post
    auto post = [&](float x, float y, float z) {
        solidBox(x, y, z, x + outSign * rSize, y + rH, z + rSize,
            0.92f, 0.92f, 0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
    };

    // Flight 1 outer edge (xInner side) going from ground to mid-landing
    for (int i = 0; i <= N * 3; i++) {
        float f = (float)i / (N * 3);
        float yL = f * yMid;
        float zL = zFront - f * (zFront - zBack);
        post(xInner, yL, zL);
    }

    // Flight 1 inner edge (midX_Inner side) — previously missing
    for (int i = 0; i <= N * 3; i++) {
        float f = (float)i / (N * 3);
        float yL = f * yMid;
        float zL = zFront - f * (zFront - zBack);
        post(midX_Inner, yL, zL);
    }

    // Flight 2 outer edge (xOuter side)
    for (int i = 0; i <= N * 3; i++) {
        float f = (float)i / (N * 3);
        float yU = yMid + f * (yTop - yMid);
        float zU = zBack + f * (zFront - zBack);
        post(xOuter, yU, zU);
    }

    // Flight 2 inner edge (midX_Outer side) — previously missing
    for (int i = 0; i <= N * 3; i++) {
        float f = (float)i / (N * 3);
        float yU = yMid + f * (yTop - yMid);
        float zU = zBack + f * (zFront - zBack);
        post(midX_Outer, yU, zU);
    }

    // Mid-landing: front edge posts (the open face, away from building)
    for (float x = xOuter; x <= xInner; x += density)
        post(x, yMid, zBack - lDepth);

    // Mid-landing: outer side posts (along Z, the exposed outer edge)
    for (float z = zBack - lDepth; z <= zBack; z += density)
        solidBox(xOuter + outSign * rSize, yMid, z,
            xOuter, yMid + rH, z + rSize,
            0.92f, 0.92f, 0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
    // NOTE: No back edge railing (against building wall) and no inner railing (stair opening)

    // Mid-landing: horizontal top rail caps — front and outer side only
    // Front cap
    solidBox(xOuter, yMid + rH - 0.02f, zBack - lDepth - 0.01f, xInner, yMid + rH + 0.02f, zBack - lDepth + 0.03f,
        0.80f, 0.80f, 0.80f, 0.70f, 0.70f, 0.70f, 0.85f, 0.85f, 0.85f);
    // Outer side cap
    solidBox(xOuter + outSign * 0.03f, yMid + rH - 0.02f, zBack - lDepth,
        xOuter, yMid + rH + 0.02f, zBack,
        0.80f, 0.80f, 0.80f, 0.70f, 0.70f, 0.70f, 0.85f, 0.85f, 0.85f);

    // Top-landing platform: X from min(xOuter,xInner) to max(xOuter,xInner), Z from 0.7 to 1.65
    float tX1 = (xOuter < xInner) ? xOuter : xInner;  // smaller X
    float tX2 = (xOuter < xInner) ? xInner : xOuter;  // larger X
    float tZ1 = 0.70f;
    float tZ2 = 1.65f;
    float pH = yTop + rH;  // top of post
    float tXout = xOuter; // the real outer X edge (left=-5.7, right=5.7)

    // Front face (z = tZ2, posts along X)
    for (float x = tX1; x <= tX2; x += density)
        solidBox(x, yTop, tZ2, x + rSize, pH, tZ2 + rSize, 0.92f, 0.92f, 0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
    solidBox(tX1, pH - 0.02f, tZ2, tX2, pH + 0.02f, tZ2 + 0.04f, 0.82f, 0.82f, 0.82f, 0.72f, 0.72f, 0.72f, 0.88f, 0.88f, 0.88f);

    // Outer side (x = xOuter, posts along Z) — outward direction
    for (float z = tZ1; z <= tZ2; z += density)
        solidBox(tXout, yTop, z, tXout + outSign * rSize, pH, z + rSize, 0.92f, 0.92f, 0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
    solidBox(tXout, pH - 0.02f, tZ1, tXout + outSign * 0.04f, pH + 0.02f, tZ2, 0.82f, 0.82f, 0.82f, 0.72f, 0.72f, 0.72f, 0.88f, 0.88f, 0.88f);
    // NOTE: No railing at z=tZ1 — that is the staircase entrance opening!

    //  ── SLOPED HANDRAILS (top rails on each flight) ─────────────────
    glColor3f(0.85f, 0.85f, 0.85f);
    glLineWidth(4.5f);
    glBegin(GL_LINES);
    // Flight 1 — outer rail
    glVertex3f(xInner, 0.0f + rH, zFront);
    glVertex3f(xInner, yMid + rH, zBack);
    // Flight 1 — inner rail
    glVertex3f(midX_Inner, 0.0f + rH, zFront);
    glVertex3f(midX_Inner, yMid + rH, zBack);
    // Flight 2 — outer rail
    glVertex3f(xOuter, yMid + rH, zBack);
    glVertex3f(xOuter, yTop + rH, zFront);
    // Flight 2 — inner rail
    glVertex3f(midX_Outer, yMid + rH, zBack);
    glVertex3f(midX_Outer, yTop + rH, zFront);
    glEnd();

    //  ── DRAIN PIPES ─────────────────────────────────────────────────
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.5f);
    glBegin(GL_LINES);
    glVertex3f(xOuter - 0.05f, 0.0f, 1.75f); glVertex3f(xOuter - 0.05f, 2.1f, 1.75f);
    glVertex3f(xInner + 0.05f, 0.0f, 1.75f); glVertex3f(xInner + 0.05f, 2.1f, 1.75f);
    glEnd();
    solidBox(xOuter - 0.12f, 0.0f, 1.70f, xOuter + 0.02f, 0.10f, 1.85f, 1.0f, 1.0f, 1.0f, 0.9f, 0.9f, 0.9f, 1.0f, 1.0f, 1.0f);
    solidBox(xInner - 0.02f, 0.0f, 1.70f, xInner + 0.12f, 0.10f, 1.85f, 1.0f, 1.0f, 1.0f, 0.9f, 0.9f, 0.9f, 1.0f, 1.0f, 1.0f);
}

//           main building                                                                                                                                                                                  
void drawBuilding()
{
    float BX1 = -4.5f, BX2 = 4.5f; // building X extents
    float BZ1 = -0.7f, BZ2 = 0.7f; // building depth
    float BZ_Back = -3.5f;
    float FX1 = BX1 - 1.25f;
    float FX2 = BX2 + 1.25f;

    //           0. FOUNDATION PLINTH (The base the whole building sits on)                
    // 1. Wide Front Part (Under stairs and main front)
    solidBox(FX1, 0.0f, -0.7f, FX2, 0.15f, 1.8f,
        0.6f, 0.62f, 0.65f, 0.6f, 0.62f, 0.65f, 0.92f, 0.60f, 0.38f);
    // 2. Narrower Back Part (Under rear sloping section)
    solidBox(BX1, 0.0f, BZ_Back - 0.2f, BX2, 0.15f, -0.7f,
        0.6f, 0.62f, 0.65f, 0.6f, 0.62f, 0.65f, 0.92f, 0.60f, 0.38f);

    // ── CONCRETE ACCESS RAMP (right side of building front) ──
    // Connects plinth (y=0.15 at z=1.8) down to ground (y=0 at z=3.5)
    // X range: 4.6 to 5.4 (narrower)
    glDisable(GL_LIGHTING);
    // Top sloped surface
    glColor3f(0.55f, 0.52f, 0.48f);
    glBegin(GL_QUADS);
    glVertex3f(4.6f, 0.15f, 1.8f);   // back-left  (plinth level)
    glVertex3f(5.4f, 0.15f, 1.8f);   // back-right (plinth level)
    glVertex3f(5.4f, 0.00f, 3.5f);   // front-right (ground)
    glVertex3f(4.6f, 0.00f, 3.5f);   // front-left  (ground)
    glEnd();
    // Front edge
    glColor3f(0.48f, 0.45f, 0.42f);
    glBegin(GL_QUADS);
    glVertex3f(4.6f, 0.00f, 3.5f);
    glVertex3f(5.4f, 0.00f, 3.5f);
    glVertex3f(5.4f, -0.02f, 3.5f);
    glVertex3f(4.6f, -0.02f, 3.5f);
    glEnd();
    // Left side triangle
    glColor3f(0.50f, 0.47f, 0.44f);
    glBegin(GL_TRIANGLES);
    glVertex3f(4.6f, 0.15f, 1.8f);
    glVertex3f(4.6f, 0.00f, 1.8f);
    glVertex3f(4.6f, 0.00f, 3.5f);
    glEnd();
    // Right side triangle
    glColor3f(0.50f, 0.47f, 0.44f);
    glBegin(GL_TRIANGLES);
    glVertex3f(5.4f, 0.15f, 1.8f);
    glVertex3f(5.4f, 0.00f, 3.5f);
    glVertex3f(5.4f, 0.00f, 1.8f);
    glEnd();
    glEnable(GL_LIGHTING);

    // Square tile pattern
    glColor3f(0.78f, 0.62f, 0.42f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float x = FX1; x <= FX2; x += 0.8f) {
        glVertex3f(x, 0.151f, -0.7f);
        glVertex3f(x, 0.151f, 1.8f);
    }
    for (float z = -0.7f; z <= 1.8f; z += 0.8f) {
        glVertex3f(FX1, 0.151f, z);
        glVertex3f(FX2, 0.151f, z);
    }
    for (float x = BX1; x <= BX2; x += 0.8f) {
        glVertex3f(x, 0.151f, BZ_Back - 0.2f);
        glVertex3f(x, 0.151f, -0.7f);
    }
    for (float z = BZ_Back - 0.2f; z <= -0.7f; z += 0.8f) {
        glVertex3f(BX1, 0.151f, z);
        glVertex3f(BX2, 0.151f, z);
    }
    glEnd();

    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f); // Shift everything up to sit ON the plinth

    // door positions (5 doors)
    float dX[5] = { -3.4f,-1.7f,0.0f,1.7f,3.4f };
    float VY1 = 2.65f, VY2 = 3.25f; // Declared here to be used in First Floor and Attic

    //           1. GROUND FLOOR (Striped Front & Sides)                                                          
    // Bottom Band (Grey)
    solidBox(BX1, 0.0f, BZ1, BX2, 0.5f, BZ2, 0.35f, 0.38f, 0.41f, 0.35f, 0.38f, 0.41f, 0.35f, 0.38f, 0.41f);
    // Top Band (White Front, Orange Side)
    solidBox(BX1, 0.5f, BZ1, BX2, 1.0f, BZ2, 0.92f, 0.92f, 0.92f, 0.92f, 0.60f, 0.38f, 0.92f, 0.60f, 0.38f);

    // Detailed doors
    for (int i = 0; i < 5; i++) {
        float angle = 0.0f;
        // Middle door (index 2) opens when we transition
        if (i == 2 && currentMode != MODE_EXTERIOR) {
            angle = targetDoorAngle;
        }
        drawDetailedDoor(dX[i], 0.0f, BZ2 + 0.002f, angle);
    }

    //           2. FIRST FLOOR (Striped Front, Orange Side)                                                          
    // Front face (Two-tone: Bottom Grey, Top White)
    // Bottom Half (Grey) - just above balcony
    glColor3f(0.35f, 0.38f, 0.41f);
    glBegin(GL_QUADS);
    glVertex3f(BX1, 1.0f, BZ2 + 0.001f); glVertex3f(BX2, 1.0f, BZ2 + 0.001f);
    glVertex3f(BX2, 1.5f, BZ2 + 0.001f); glVertex3f(BX1, 1.5f, BZ2 + 0.001f);
    glEnd();
    // Top Half (White)
    glColor3f(0.92f, 0.92f, 0.92f);
    glBegin(GL_QUADS);
    glVertex3f(BX1, 1.5f, BZ2 + 0.001f); glVertex3f(BX2, 1.5f, BZ2 + 0.001f);
    glVertex3f(BX2, VY1, BZ2 + 0.001f); glVertex3f(BX1, VY1, BZ2 + 0.001f); // Extended to VY1
    glEnd();
    // Solid Orange Sides & Back
    solidBox(BX1, 1.0f, BZ1, BX2, VY1, BZ2, 0.35f, 0.38f, 0.41f, 0.88f, 0.72f, 0.52f, 0.82f, 0.66f, 0.46f);

    // Detailed doors (Balcony level)
    for (int i = 0; i < 5; i++) {
        drawDetailedDoor(dX[i], 1.02f, BZ2 + 0.002f);
    }

    //           3. UPPER ATTIC (Front Face & Sides)                                                                                  
    float VX1 = BX1, VX2 = BX2;
    float VZ1 = -0.5f, VZ2 = 0.55f;
    solidBox(VX1, VY1, VZ1, VX2, VY2, VZ2, 0.88f, 0.72f, 0.52f, 0.88f, 0.72f, 0.52f, 0.82f, 0.66f, 0.46f);

    // Row of clusters of tiny breezeway holes
    glColor3f(0.05f, 0.05f, 0.05f);
    for (float cx = VX1 + 0.4f; cx < VX2 - 0.4f; cx += 1.0f) {
        for (float dx = -0.3f; dx <= 0.3f; dx += 0.15f) {
            for (float dy = 0.25f; dy <= 0.45f; dy += 0.15f) {
                glBegin(GL_QUADS);
                glVertex3f(cx + dx - 0.04f, VY1 + dy - 0.04f, VZ2 + 0.01f);
                glVertex3f(cx + dx + 0.04f, VY1 + dy - 0.04f, VZ2 + 0.01f);
                glVertex3f(cx + dx + 0.04f, VY1 + dy + 0.04f, VZ2 + 0.01f);
                glVertex3f(cx + dx - 0.04f, VY1 + dy + 0.04f, VZ2 + 0.01f);
                glEnd();
            }
        }
    }

    //           4. LARGE REAR SLOPING SECTION (The "Feature")                                                       
    BZ_Back = -3.5f;
    float BY_Back = 1.65f; // Taller back wall to fit windows properly

    // Rear Roof Slab (Silver/Grey)
    glColor3f(0.72f, 0.72f, 0.75f);
    glBegin(GL_QUADS);
    glVertex3f(BX1 - 0.1f, VY2, VZ1);      glVertex3f(BX2 + 0.1f, VY2, VZ1);
    glVertex3f(BX2 + 0.1f, BY_Back, BZ_Back); glVertex3f(BX1 - 0.1f, BY_Back, BZ_Back);
    glEnd();

    // Rear Trapezoidal Side Walls (Orange-Tan)
    glColor3f(0.88f, 0.72f, 0.52f);
    // Left Side
    glBegin(GL_QUADS);
    glVertex3f(BX1, 0.0f, VZ1); glVertex3f(BX1, 0.0f, BZ_Back);
    glVertex3f(BX1, BY_Back, BZ_Back); glVertex3f(BX1, VY2, VZ1);
    glEnd();
    // Right Side
    glBegin(GL_QUADS);
    glVertex3f(BX2, 0.0f, VZ1); glVertex3f(BX2, 0.0f, BZ_Back);
    glVertex3f(BX2, BY_Back, BZ_Back); glVertex3f(BX2, VY2, VZ1);
    glEnd();
    // Back Wall (Pinkish-Tan / Peach)
    glColor3f(0.95f, 0.76f, 0.68f);
    glBegin(GL_QUADS);
    glVertex3f(BX1, 0.0f, BZ_Back); glVertex3f(BX2, 0.0f, BZ_Back);
    glVertex3f(BX2, BY_Back, BZ_Back); glVertex3f(BX1, BY_Back, BZ_Back);
    glEnd();

    // Back Windows and Columns (Repeated Segments)
    // Draw 5 segments across the back
    float totalBackWidth = BX2 - BX1;
    float segWidth = totalBackWidth / 5.0f;

    for (int s = 0; s < 5; s++) {
        float colX = BX1 + (s * segWidth);
        float segX = colX + (segWidth * 0.5f);

        // Structural Column (Dark Grey)
        glColor3f(0.32f, 0.32f, 0.35f);
        solidBox(colX - 0.08f, 0.0f, BZ_Back - 0.08f, colX + 0.08f, BY_Back, BZ_Back + 0.05f,
            0.32f, 0.32f, 0.35f, 0.28f, 0.28f, 0.31f, 0.35f, 0.35f, 0.38f);

        // Vertical Drain Pipe (White)
        glColor3f(0.95f, 0.95f, 0.95f);
        solidBox(colX + 0.06f, 0.0f, BZ_Back - 0.09f, colX + 0.10f, BY_Back + 0.05f, BZ_Back - 0.03f, 0.95f, 0.95f, 0.95f, 0.9f, 0.9f, 0.9f, 0.95f, 0.95f, 0.95f);

        // Ceiling Overhang
        solidBox(colX + 0.1f, BY_Back - 0.05f, BZ_Back - 0.08f, colX + segWidth - 0.1f, BY_Back, BZ_Back, 0.95f, 0.95f, 0.95f, 0.9f, 0.9f, 0.9f, 0.95f, 0.95f, 0.95f);

        // Final Closing Column at the very end
        if (s == 4) {
            float lastX = colX + segWidth;
            glColor3f(0.32f, 0.32f, 0.35f);
            solidBox(lastX - 0.08f, 0.0f, BZ_Back - 0.08f, lastX + 0.08f, BY_Back, BZ_Back + 0.05f, 0.32f, 0.32f, 0.35f, 0.28f, 0.28f, 0.31f, 0.35f, 0.35f, 0.38f);
        }

        // Function to draw each window in the segment
        auto drawSegmentWindow = [&](float x, float y, bool vertical) {
            // Window Sill (Smaller)
            solidBox(x - 0.58f, y - 0.02f, BZ_Back - 0.04f, x + 0.58f, y + 0.02f, BZ_Back, 0.9f, 0.9f, 0.9f, 0.8f, 0.8f, 0.8f, 0.95f, 0.95f, 0.95f);

            // Multi-Pane Glass (With Specular Highlight)
            float pw = 1.1f / 3.0f;
            for (int i = 0; i < 3; i++) {
                float px = x - 0.55f + (i * pw);
                if ((i + s) % 2 == 0) glColor3f(0.2f, 0.3f, 0.4f);
                else glColor3f(0.55f, 0.65f, 0.75f);

                glBegin(GL_QUADS);
                glNormal3f(0, 0, -1);
                glVertex3f(px + 0.01f, y + 0.03f, BZ_Back - 0.01f); glVertex3f(px + pw - 0.01f, y + 0.03f, BZ_Back - 0.01f);
                glVertex3f(px + pw - 0.01f, y + 0.38f, BZ_Back - 0.01f); glVertex3f(px + 0.01f, y + 0.38f, BZ_Back - 0.01f);
                glEnd();
            }

            // Grills (Tightly aligned with smaller glass)
            glColor3f(0.1f, 0.1f, 0.1f);
            glLineWidth(1.2f);
            glBegin(GL_LINES);
            if (vertical) {
                for (float i = -0.54f; i <= 0.54f; i += 0.08f) {
                    glVertex3f(x + i, y + 0.03f, BZ_Back - 0.015f); glVertex3f(x + i, y + 0.40f, BZ_Back - 0.015f);
                }
            }
            else {
                for (float i = 0.10f; i <= 0.35f; i += 0.12f) {
                    glVertex3f(x - 0.55f, y + i, BZ_Back - 0.015f); glVertex3f(x + 0.55f, y + i, BZ_Back - 0.015f);
                }
            }
            glEnd();
        };

        drawSegmentWindow(segX, 0.15f, true);  // Ground floor
        drawSegmentWindow(segX, 0.95f, false); // First floor
    }
    // Final closing column on the right
    solidBox(BX2 - 0.35f, 0.0f, BZ_Back - 0.01f, BX2, BY_Back, BZ_Back + 0.05f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f);

    //           2. SECOND FLOOR BALCONY                                                                                                                
    solidBox(BX1, 1.0f, 0.7f, BX2, 1.08f, 1.65f,
        0.72f, 0.72f, 0.72f, 0.58f, 0.58f, 0.58f, 0.66f, 0.66f, 0.66f);

    // --- 2b. BALCONY RAILINGS ---
    float bDensity = 0.06f;
    float bRSize = 0.015f;
    glColor3f(0.9f, 0.9f, 0.9f);
    // Front railing
    for (float x = BX1; x <= BX2; x += bDensity) {
        solidBox(x, 1.08f, 1.63f, x + bRSize, 1.58f, 1.63f + bRSize, 0.92f, 0.92f, 0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
    }
    // Top Bar (Front only)
    solidBox(BX1, 1.55f, 1.63f, BX2, 1.58f, 1.63f + bRSize, 0.8f, 0.8f, 0.8f, 0.7f, 0.7f, 0.7f, 0.85f, 0.85f, 0.85f);

    //           4b. SIDE WALL CONCRETE GRID (The "Cross" pattern)                                     
    glColor3f(0.75f, 0.65f, 0.55f);
    // Horizontal beams
    solidBox(BX1 - 0.01f, 1.0f, BZ_Back, BX1 + 0.05f, 1.15f, 0.7f, 0.7f, 0.6f, 0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);
    solidBox(BX2 - 0.05f, 1.0f, BZ_Back, BX2 + 0.01f, 1.15f, 0.7f, 0.7f, 0.6f, 0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);
    // Vertical beams
    solidBox(BX1 - 0.01f, 0.0f, -2.0f, BX1 + 0.05f, 2.0f, -1.85f, 0.7f, 0.6f, 0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);
    solidBox(BX2 - 0.05f, 0.0f, -2.0f, BX2 + 0.01f, 2.0f, -1.85f, 0.7f, 0.6f, 0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);



    //           4. MAIN SLANTED CORRUGATED ROOF                                                                                                 
    float RX1 = BX1, RX2 = BX2;
    float RY_Back = 2.75f, RY_Front = 2.40f; // Raised to create the vertical gap above the canopy
    float RZ_Back = -0.5f, RZ_Front = 2.0f;

    // Corrugated stripes (slanted)
    bool dark = false;
    for (float x = RX1; x < RX2; x += 0.12f) {
        float x_next = x + 0.12f;
        if (x_next > RX2) x_next = RX2;
        glColor3f(dark ? 0.65f : 0.75f, dark ? 0.65f : 0.75f, dark ? 0.68f : 0.78f);
        dark = !dark;
        glBegin(GL_QUADS);
        glVertex3f(x, RY_Back, RZ_Back);  glVertex3f(x_next, RY_Back, RZ_Back);
        glVertex3f(x_next, RY_Front, RZ_Front); glVertex3f(x, RY_Front, RZ_Front);
        glEnd();
    }

    //           5. STAIRCASE HORIZONTAL GABLE CANOPIES with TRUSSES                               
    // Ridge runs horizontally (parallel to building front)
    float CY_P = 2.15f;   // Ridge peak
    float CY_E = 1.70f;   // Eave height (Restored)
    float CZ_B = -0.5f, CZ_F = 2.0f;
    float midZ = (CZ_B + CZ_F) * 0.5f;

    auto drawHorizontalGable = [&](float xOut, float xIn) {
        bool dark = false;
        for (float x = xOut; x < xIn; x += 0.12f) {
            float x_next = x + 0.12f;
            if (x_next > xIn) x_next = xIn;
            glColor3f(dark ? 0.65f : 0.75f, dark ? 0.65f : 0.75f, dark ? 0.68f : 0.78f);
            dark = !dark;
            // Back slope
            glBegin(GL_QUADS);
            glVertex3f(x, CY_E, CZ_B); glVertex3f(x_next, CY_E, CZ_B);
            glVertex3f(x_next, CY_P, midZ); glVertex3f(x, CY_P, midZ);
            glEnd();
            // Front slope
            glBegin(GL_QUADS);
            glVertex3f(x, CY_P, midZ); glVertex3f(x_next, CY_P, midZ);
            glVertex3f(x_next, CY_E, CZ_F); glVertex3f(x, CY_E, CZ_F);
            glEnd();
        }
        // Metal Edge Capping
        glColor3f(0.6f, 0.62f, 0.65f);
        glLineWidth(3.0f);
        glBegin(GL_LINE_STRIP);
        glVertex3f(xOut, CY_E, CZ_F + 0.01f); glVertex3f(xOut, CY_P, midZ); glVertex3f(xOut, CY_E, CZ_B - 0.01f);
        glEnd();
    };

    drawHorizontalGable(-6.0f, -4.5f); // Connected to wall
    drawHorizontalGable(4.5f, 6.0f);

    // Metal Truss (Horizontal alignment)
    glColor3f(0.35f, 0.35f, 0.35f);
    glLineWidth(1.5f);
    auto drawHTruss = [&](float xOut, float xIn) {
        for (float x = xOut; x <= xIn; x += 0.5f) {
            glBegin(GL_LINES);
            // Main triangle
            glVertex3f(x, CY_E, CZ_B); glVertex3f(x, CY_E, CZ_F);
            glVertex3f(x, CY_E, CZ_B); glVertex3f(x, CY_P, midZ);
            glVertex3f(x, CY_E, CZ_F); glVertex3f(x, CY_P, midZ);
            // Diagonal bracing
            glVertex3f(x, CY_E, (CZ_B + midZ) * 0.5f); glVertex3f(x, CY_P, midZ);
            glVertex3f(x, CY_E, (CZ_F + midZ) * 0.5f); glVertex3f(x, CY_P, midZ);
            glEnd();
        }
    };
    drawHTruss(-6.0f, -4.5f);
    drawHTruss(4.5f, 6.0f);

    //           6. SUPPORT PILLARS (Ending at roof level)                                                             
    glColor3f(0.55f, 0.56f, 0.58f); // Weathered Concrete Grey
    glLineWidth(4.0f);
    // Main front pillars
    float pX[6] = { -4.5f,-2.7f,-0.9f,0.9f,2.7f,4.5f };
    for (int i = 0; i < 6; i++) {
        glBegin(GL_LINES);
        glVertex3f(pX[i], 0.0f, 1.63f);
        glVertex3f(pX[i], 2.45f, 1.63f); // Corrected height to meet the new higher roof
        glEnd();
    }
    // Outer staircase pillars (Restored to 1.70f height)
    glBegin(GL_LINES);
    // Front pillars
    glVertex3f(-4.55f, 0.0f, 1.9f); glVertex3f(-4.55f, CY_E, 1.9f);
    glVertex3f(4.55f, 0.0f, 1.9f); glVertex3f(4.55f, CY_E, 1.9f);
    glVertex3f(-5.95f, 0.0f, 1.9f); glVertex3f(-5.95f, CY_E, 1.9f);
    glVertex3f(5.95f, 0.0f, 1.9f); glVertex3f(5.95f, CY_E, 1.9f);
    // Back pillars
    glVertex3f(-4.55f, 0.0f, -0.4f); glVertex3f(-4.55f, CY_E, -0.4f);
    glVertex3f(4.55f, 0.0f, -0.4f); glVertex3f(4.55f, CY_E, -0.4f);
    glVertex3f(-5.95f, 0.0f, -0.4f); glVertex3f(-5.95f, CY_E, -0.4f);
    glVertex3f(5.95f, 0.0f, -0.4f); glVertex3f(5.95f, CY_E, -0.4f);
    glEnd();
    glLineWidth(1.0f);

    //           7. STAIRS (U-shaped / Dog-leg)                                                                                                 
    drawStaircase(-5.7f, -4.5f, 0.7f, -0.3f); // One step back
    drawStaircase(5.7f, 4.5f, 0.7f, -0.3f);

    // Draw students on the balcony
    drawStudents();

    if (currentMode == MODE_TRANSITION || currentMode == MODE_TRANSITION_BACK) {
        drawClassroomInterior();
    }

    glPopMatrix(); // End of vertical shift
}

//           display                                                                                                                                                                                                    
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // ULTIMATE LOCKDOWN: Apply clamps BEFORE any rendering math
    if (currentMode == MODE_CLASSROOM) {
        if (posX < -3.3f) posX = -3.3f;
        if (posX > 3.3f) posX = 3.3f;
        if (posZ < -6.3f) posZ = -6.3f;
        if (posZ > 0.45f) posZ = 0.45f;
        if (camHeight < 0.4f) camHeight = 0.4f; 
        if (camHeight > 1.8f) camHeight = 1.8f; // Human height cap
        zoom = 0.0f;
    }

    float p = transitionProgress;
    float easeP = p * p * (3.0f - 2.0f * p); // smooth step interpolation

    float currentCamHeight = camHeight;
    float currentZoom = zoom;
    float currentTilt = 12.0f;

    if (currentMode == MODE_TRANSITION || currentMode == MODE_TRANSITION_BACK) {
        float targetCamHeight = 0.6f; 
        float targetZoom = enteringStudentZ + 2.0f; 
        float targetTilt = 0.0f; 
        
        currentCamHeight = camHeight * (1.0f - easeP) + targetCamHeight * easeP;
        currentZoom = zoom * (1.0f - easeP) + targetZoom * easeP;
        currentTilt = 12.0f * (1.0f - easeP) + targetTilt * easeP;

        glTranslatef(0.0f, -currentCamHeight, -currentZoom);
        glRotatef(currentTilt, 1.0f, 0.0f, 0.0f);
        glPushMatrix();
        glRotatef(autoAngle, 0.0f, 1.0f, 0.0f);
    } else if (currentMode == MODE_CLASSROOM) {
        glRotatef(autoAngle, 0.0f, 1.0f, 0.0f);
        glTranslatef(-posX, -currentCamHeight, -posZ);
        glPushMatrix();
    } else {
        glTranslatef(0.0f, -currentCamHeight, -currentZoom);
        glRotatef(12.0f, 1.0f, 0.0f, 0.0f);
        glPushMatrix();
        glRotatef(autoAngle, 0.0f, 1.0f, 0.0f);
    }

    if (currentMode == MODE_CLASSROOM) {
        glPushMatrix();
        glTranslatef(0.0f, 0.15f, 0.0f);
        drawClassroomInterior();
        glPopMatrix();
    } else {
        drawSky();
        drawEarth();
        drawCar(carProgress, 0.0f, 14.0f, 90.0f); // Moving black car
        drawBuilding();
    }
    
    glPopMatrix();
    glutSwapBuffers();
}

void applySkySettings()
{
    if (isNight) {
        // Deep night sky
        glClearColor(0.05f, 0.07f, 0.15f, 1.0f);
        float ambient[] = { 0.08f, 0.08f, 0.15f, 1.0f };
        float diffuse[] = { 0.15f, 0.15f, 0.25f, 1.0f };
        float lightPos[] = { 0.0f, 25.0f, 10.0f, 0.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    }
    else {
        // Daytime sky blue
        glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
        float ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
        float diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
        float lightPos[] = { -15.0f, 25.0f, 25.0f, 0.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    }
}

void init()
{
    qobj = gluNewQuadric();
    gluQuadricNormals(qobj, GLU_SMOOTH);

    glClearColor(0.6f, 0.8f, 0.92f, 1.0f); // Will be overridden by applySkySettings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Lighting setup
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL); // Allow glColor to set material colors

    float lightPos[] = { -15.0f, 25.0f, 25.0f, 0.0f }; // Moved to side/front for deeper shadows
    float ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    float diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    float specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    // Material properties for shininess
    float specMat[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specMat);
    glMateriali(GL_FRONT, GL_SHININESS, 64);

    // Smoothing/Anti-aliasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    applySkySettings(); // Apply initial day lighting
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'w' || key == 'W') camHeight += 0.2f;
    if (key == 's' || key == 'S') camHeight -= 0.2f;

    // Day / Night toggle
    if (key == 'n' || key == 'N') {
        isNight = !isNight;
        applySkySettings();
    }
    
    // Start Cinematic Classroom Transition
    if (key == 'g' || key == 'G') {
        if (currentMode == MODE_EXTERIOR) {
            currentMode = MODE_TRANSITION;
            transitionProgress = 0.0f;
            targetDoorAngle = 0.0f;
            enteringStudentZ = 1.35f;
            enteringStudentX = 0.0f;
        }
    }
    
    // Return to Normal Scene
    if (key == 'b' || key == 'B') {
        if (currentMode == MODE_CLASSROOM) {
            currentMode = MODE_TRANSITION_BACK;
            // Keep transitionProgress at 1.0f, it will animate backwards
        }
    }

    // Limits
    if (camHeight < -1.0f) camHeight = -1.0f;
    if (camHeight > 5.0f)  camHeight = 5.0f;

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y)
{
    if (currentMode == MODE_CLASSROOM) {
        float speed = 0.2f;
        float rad = autoAngle * 3.14159f / 180.0f;
        if (key == GLUT_KEY_UP) {
            posX += sinf(rad) * speed;
            posZ -= cosf(rad) * speed;
        }
        if (key == GLUT_KEY_DOWN) {
            posX -= sinf(rad) * speed;
            posZ += cosf(rad) * speed;
        }
        
        // STRICT BOUNDARIES
        if (posX < -3.3f) posX = -3.3f;
        if (posX > 3.3f) posX = 3.3f;
        if (posZ < -6.3f) posZ = -6.3f;
        if (posZ > 1.3f) posZ = 1.3f;
    } else {
        if (key == GLUT_KEY_UP)    zoom -= 0.5f; // Zoom In
        if (key == GLUT_KEY_DOWN)  zoom += 0.5f; // Zoom Out
    }

    if (key == GLUT_KEY_RIGHT) autoAngle += 5.0f;
    if (key == GLUT_KEY_LEFT)  autoAngle -= 5.0f;

    if (currentMode == MODE_EXTERIOR) {
        if (zoom < 4.0f) zoom = 4.0f;
        if (zoom > 30.0f) zoom = 30.0f;
    }

    glutPostRedisplay();
}

void idle()
{
    // Drift clouds slowly across the sky
    cloudOffset += 0.01f;
    if (cloudOffset > 100.0f) cloudOffset = 0.0f;

    // Move students (reduced speed)
    studentWalkAngle += 0.2f;
    if (studentWalkAngle > 360.0f) studentWalkAngle -= 360.0f;

    // Move car
    carProgress += 0.1f;
    if (carProgress > 45.0f) carProgress = -45.0f;

    if (currentMode == MODE_TRANSITION) {
        // Stop spinning and align building to face camera
        autoAngle = autoAngle * 0.95f;
        if (fabs(autoAngle) < 0.1f) autoAngle = 0.0f;

        transitionProgress += 0.003f; // controls cinematic speed
        if (transitionProgress >= 1.0f) {
            transitionProgress = 1.0f;
            currentMode = MODE_CLASSROOM; 
            // Hand over control to user at the current spot
            posX = 0.0f;
            posZ = enteringStudentZ + 2.0f;
            camHeight = 0.6f;
            autoAngle = 0.0f;
        }
        
        // Open door logic (door is at X=0, opens starting at progress 0.2, fully open at 0.5)
        if (transitionProgress > 0.2f) {
            float doorP = (transitionProgress - 0.2f) / 0.3f;
            if (doorP > 1.0f) doorP = 1.0f;
            // Smooth ease
            float dEase = doorP * doorP * (3.0f - 2.0f * doorP);
            targetDoorAngle = dEase * -90.0f; // opens inwards
        } else {
            targetDoorAngle = 0.0f;
        }
        
        // Entering student logic (starts moving at progress 0.4, reaches inside by 1.0)
        if (transitionProgress > 0.4f) {
            float walkP = (transitionProgress - 0.4f) / 0.6f;
            // Student starts at Z=1.35, moves into the building Z=-2.5
            enteringStudentZ = 1.35f - (walkP * 3.85f);
            enteringStudentX = 0.0f;
        } else {
            enteringStudentZ = 1.35f;
            enteringStudentX = 0.0f;
        }
    } else if (currentMode == MODE_TRANSITION_BACK) {
        transitionProgress -= 0.003f; // reverse speed
        if (transitionProgress <= 0.0f) {
            transitionProgress = 0.0f;
            currentMode = MODE_EXTERIOR; 
        }
        
        // Door logic in reverse
        if (transitionProgress > 0.2f) {
            float doorP = (transitionProgress - 0.2f) / 0.3f;
            if (doorP > 1.0f) doorP = 1.0f;
            float dEase = doorP * doorP * (3.0f - 2.0f * doorP);
            targetDoorAngle = dEase * -90.0f;
        } else {
            targetDoorAngle = 0.0f;
        }
        
        // Student logic in reverse
        if (transitionProgress > 0.4f) {
            float walkP = (transitionProgress - 0.4f) / 0.6f;
            enteringStudentZ = 1.35f - (walkP * 3.85f);
            enteringStudentX = 0.0f;
        } else {
            enteringStudentZ = 1.35f;
            enteringStudentX = 0.0f;
        }
    }

    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("School Building - 360 View | N = Day/Night");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);   // Handle W/S/N keys
    glutSpecialFunc(specialKeys); // Handle arrow keys
    glutIdleFunc(idle);           // Cloud drift animation
    glutMainLoop();
    return 0;
}