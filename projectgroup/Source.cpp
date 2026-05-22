#include <windows.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// GLOBAL PROJECT CONSTANTS & CONFIGURATION
// ----------------------------------------------------------------------------

namespace CampusConfig {
// Ground Dimensions
const float GROUND_SIZE = 100.0f;
const float GROUND_Y = -0.02f;

// Building Dimensions
const float BUILDING_WIDTH = 9.0f;
const float BUILDING_HEIGHT = 5.0f;
const float BUILDING_DEPTH = 5.5f;

// Road Configuration
const float ROAD_Z_START = 12.0f;
const float ROAD_Z_END = 17.0f;
const float ROAD_Y = -0.015f;

// Lighting Colors (Day)
const float DAY_SKY_R = 0.53f;
const float DAY_SKY_G = 0.81f;
const float DAY_SKY_B = 0.92f;

// Lighting Colors (Night)
const float NIGHT_SKY_R = 0.05f;
const float NIGHT_SKY_G = 0.05f;
const float NIGHT_SKY_B = 0.15f;

// Material Palettes
const float COLOR_ASPHALT[] = {0.22f, 0.22f, 0.25f};
const float COLOR_GRASS[] = {0.25f, 0.45f, 0.15f};
const float COLOR_CONCRETE[] = {0.75f, 0.75f, 0.72f};
} // namespace CampusConfig

// ----------------------------------------------------------------------------
// PROJECT DATA STRUCTURES
// ----------------------------------------------------------------------------

/**
 * @class CampusManager
 * @brief Manages the global state and configuration of the university campus
 * simulation
 */
class CampusManager {
public:
  // Application Modes
  enum AppMode {
    MODE_FREE_LOOK = 0,
    MODE_TRANSITION = 1,
    MODE_CLASSROOM = 2,
    MODE_TRANSITION_BACK = 3,
    MODE_MORNING_CINEMATIC = 4
  };

  // Singleton instance access
  static CampusManager &getInstance() {
    static CampusManager instance;
    return instance;
  }

  // State Variables
  AppMode currentMode;
  bool isNight;
  float carProgress;
  float studentWalkAngle;
  float transitionProgress;

  // View State
  float camHeight;
  float zoom;
  float autoAngle;

  // Interior Coordinates
  float posX, posZ;

private:
  // Private constructor for Singleton
  CampusManager()
      : currentMode(MODE_FREE_LOOK), isNight(false), carProgress(-50.0f),
        studentWalkAngle(0.0f), transitionProgress(0.0f), camHeight(5.0f),
        zoom(25.0f), autoAngle(0.0f), posX(0.0f), posZ(0.0f) {}
};

// Global pointer for easier access
#define CAMPUS CampusManager::getInstance()

// ----------------------------------------------------------------------------
// FORWARD DECLARATIONS (DETAILED)
// ----------------------------------------------------------------------------

GLUquadricObj *qobj = NULL;
void drawDetailedDoor(float x, float y, float z, float openAngle);
void drawStudent(float x, float y, float z, float angle, float walkPhase,
                 bool isSeated, int variant, bool isTeaching);
void drawClassroomInterior();

float autoAngle = 0.0f;
float zoom = 15.0f;
float camHeight = 1.2f; // Elevated front view height

// Day/Night & Sky
bool isNight = false;
float cloudOffset = 0.0f;      // clouds drift over time
float sunMoonAngle = 30.0f;    // position of sun/moon in the sky
float studentWalkAngle = 0.0f; // students walking on the balcony

// Cinematic State Machine
enum AppMode {
  MODE_EXTERIOR,
  MODE_TRANSITION,
  MODE_CLASSROOM,
  MODE_TRANSITION_BACK,
  MODE_MORNING_CINEMATIC
};
AppMode currentMode = MODE_EXTERIOR;

float transitionProgress = 0.0f; // 0.0 to 1.0
float targetDoorAngle = 0.0f;    // Center door open angle
float enteringStudentZ = 1.35f;  // Student walking in Z pos
float enteringStudentX = 0.0f;   // Student walking in X pos
float carProgress = -40.0f;      // Car animation position

// Cinematic Morning Actors & State
float morningProgress = 0.0f;
float actStudentX = 0.0f, actStudentZ = 12.0f, actStudentAngle = 180.0f;
float actTeacherX = 0.0f, actTeacherZ = 12.0f;
bool isStudentSeated = false;
bool isTeacherTeaching = false;

// Waking Intro Cinematic & Bird variables
float introTextAlpha = 0.0f;
float focusBlurAmount = 0.0f;
float birdProgress = 0.0f;

// Welcome / Instruction Page
bool showWelcomePage = true;

// First-person classroom navigation
float posX = 0.0f, posZ = 0.0f;

// ---------------------------------------------------------------------------
// HELPER: Render a string at a given 2D raster position (assumes ortho mode)
// ---------------------------------------------------------------------------
void renderBitmapString(float x, float y, void *font, const char *str) {
  glRasterPos2f(x, y);
  for (const char *c = str; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

// ---------------------------------------------------------------------------
// STARTUP WELCOME & INSTRUCTION PAGE OVERLAY
// Draws a premium translucent overlay with project info and key bindings.
// ---------------------------------------------------------------------------
void drawWelcomePage() {
  if (!showWelcomePage) return;

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Switch to 2D orthographic projection (800x600 logical pixels)
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 800, 0, 600);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // --- 1. Full-screen dark translucent backdrop ---
  glColor4f(0.0f, 0.0f, 0.05f, 0.75f);
  glBegin(GL_QUADS);
  glVertex2f(0, 0); glVertex2f(800, 0);
  glVertex2f(800, 600); glVertex2f(0, 600);
  glEnd();

  // --- 2. Central card panel (slightly lighter) ---
  float cardL = 120.0f, cardR = 680.0f;
  float cardB = 60.0f, cardT = 550.0f;
  glColor4f(0.08f, 0.10f, 0.15f, 0.85f);
  glBegin(GL_QUADS);
  glVertex2f(cardL, cardB); glVertex2f(cardR, cardB);
  glVertex2f(cardR, cardT); glVertex2f(cardL, cardT);
  glEnd();

  // Thin gold border around the card
  glColor4f(0.85f, 0.70f, 0.30f, 0.9f);
  glLineWidth(2.0f);
  glBegin(GL_LINE_LOOP);
  glVertex2f(cardL, cardB); glVertex2f(cardR, cardB);
  glVertex2f(cardR, cardT); glVertex2f(cardL, cardT);
  glEnd();

  // --- 3. Title ---
  glColor3f(0.95f, 0.85f, 0.35f); // Gold
  renderBitmapString(195, 510, GLUT_BITMAP_TIMES_ROMAN_24, "WOLKITE UNIVERSITY");
  glColor3f(0.85f, 0.85f, 0.90f); // Silver-white
  renderBitmapString(175, 480, GLUT_BITMAP_HELVETICA_18, "3D Campus Simulation  -  OpenGL / GLUT");

  // --- 4. Thin separator line ---
  glColor4f(0.5f, 0.5f, 0.6f, 0.6f);
  glBegin(GL_LINES);
  glVertex2f(160, 465); glVertex2f(640, 465);
  glEnd();

  // --- 5. Project description ---
  glColor3f(0.75f, 0.80f, 0.85f);
  renderBitmapString(160, 438, GLUT_BITMAP_HELVETICA_12,
    "This project presents a high-fidelity 3D reconstruction of the Wolkite");
  renderBitmapString(160, 420, GLUT_BITMAP_HELVETICA_12,
    "University campus, featuring realistic day/night cycles, cinematic");
  renderBitmapString(160, 402, GLUT_BITMAP_HELVETICA_12,
    "animations, a detailed classroom interior, and procedural vegetation.");

  // --- 6. Separator ---
  glColor4f(0.5f, 0.5f, 0.6f, 0.6f);
  glBegin(GL_LINES);
  glVertex2f(160, 385); glVertex2f(640, 385);
  glEnd();

  // --- 7. Section header ---
  glColor3f(0.95f, 0.85f, 0.35f);
  renderBitmapString(310, 362, GLUT_BITMAP_HELVETICA_18, "CONTROLS");

  // --- 8. Key bindings table ---
  float keyX = 200.0f;  // Key label column
  float descX = 310.0f; // Description column
  float rowY = 332.0f;
  float rowH = 28.0f;

  // Row: M
  glColor3f(0.40f, 0.90f, 0.40f); // Green key highlight
  renderBitmapString(keyX, rowY, GLUT_BITMAP_9_BY_15, "[  M  ]");
  glColor3f(0.85f, 0.85f, 0.90f);
  renderBitmapString(descX, rowY, GLUT_BITMAP_HELVETICA_12,
    "Start Cinematic Morning Scene (sunrise, walk-in, lecture)");
  rowY -= rowH;

  // Row: U
  glColor3f(0.40f, 0.70f, 1.0f); // Blue key highlight
  renderBitmapString(keyX, rowY, GLUT_BITMAP_9_BY_15, "[  U  ]");
  glColor3f(0.85f, 0.85f, 0.90f);
  renderBitmapString(descX, rowY, GLUT_BITMAP_HELVETICA_12,
    "Campus Overview (return to exterior wide view)");
  rowY -= rowH;

  // Row: G
  glColor3f(1.0f, 0.65f, 0.25f); // Orange key highlight
  renderBitmapString(keyX, rowY, GLUT_BITMAP_9_BY_15, "[  G  ]");
  glColor3f(0.85f, 0.85f, 0.90f);
  renderBitmapString(descX, rowY, GLUT_BITMAP_HELVETICA_12,
    "Go Inside Classroom (teleport to student perspective)");
  rowY -= rowH;

  // Row: N
  glColor3f(0.70f, 0.50f, 1.0f); // Purple key highlight
  renderBitmapString(keyX, rowY, GLUT_BITMAP_9_BY_15, "[  N  ]");
  glColor3f(0.85f, 0.85f, 0.90f);
  renderBitmapString(descX, rowY, GLUT_BITMAP_HELVETICA_12,
    "Toggle Day / Night Mode");
  rowY -= rowH;

  // Row: B
  glColor3f(1.0f, 0.45f, 0.45f); // Red key highlight
  renderBitmapString(keyX, rowY, GLUT_BITMAP_9_BY_15, "[  B  ]");
  glColor3f(0.85f, 0.85f, 0.90f);
  renderBitmapString(descX, rowY, GLUT_BITMAP_HELVETICA_12,
    "Back to Normal Scene (instant pop out)");
  rowY -= rowH;

  // Row: W / S
  glColor3f(0.85f, 0.85f, 0.90f);
  renderBitmapString(keyX, rowY, GLUT_BITMAP_9_BY_15, "[ W/S ]");
  renderBitmapString(descX, rowY, GLUT_BITMAP_HELVETICA_12,
    "Raise / Lower Camera Height");
  rowY -= rowH;

  // Row: Arrow Keys
  glColor3f(0.85f, 0.85f, 0.90f);
  renderBitmapString(keyX, rowY, GLUT_BITMAP_9_BY_15, "[Arrows]");
  renderBitmapString(descX, rowY, GLUT_BITMAP_HELVETICA_12,
    "Rotate Camera / Zoom In-Out / Walk (inside class)");
  rowY -= rowH;

  // --- 9. Bottom separator ---
  glColor4f(0.5f, 0.5f, 0.6f, 0.6f);
  glBegin(GL_LINES);
  glVertex2f(160, rowY + 10); glVertex2f(640, rowY + 10);
  glEnd();

  // --- 10. Call to action ---
  glColor3f(0.60f, 0.65f, 0.70f);
  renderBitmapString(225, rowY - 15, GLUT_BITMAP_HELVETICA_12,
    "Press any key to start exploring the campus ...");

  // --- 11. Footer credits ---
  glColor3f(0.40f, 0.42f, 0.48f);
  renderBitmapString(270, 80, GLUT_BITMAP_HELVETICA_12,
    "Computer Graphics Group Project - 2026");

  // Restore 3D matrices
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
}

//           helpers
void solidBox(float x1, float y1, float z1, float x2, float y2, float z2,
              float rf, float gf, float bf, // front colour
              float rs, float gs, float bs, // side colour
              float rt, float gt, float bt) // top colour
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
// PROCEDURAL TREE GENERATION ALGORITHMS
// These functions use purely custom mathematical geometry for high-fidelity 3D
// tree generation.

// Custom geometric helper for branches to increase detail and avoid GLU
// overhead
void drawCustomBranch(float radiusBase, float radiusTop, float length,
                      int slices, int stacks) {
  float angleStep = 6.2831853f / slices;
  float heightStep = length / stacks;
  for (int j = 0; j < stacks; j++) {
    float h1 = j * heightStep;
    float h2 = (j + 1) * heightStep;
    float r1 = radiusBase - (radiusBase - radiusTop) * ((float)j / stacks);
    float r2 =
        radiusBase - (radiusBase - radiusTop) * ((float)(j + 1) / stacks);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
      float angle = i * angleStep;
      float nx = cosf(angle);
      float nz = sinf(angle);
      glNormal3f(nx, 0.0f, nz);
      glVertex3f(nx * r1, h1, nz * r1);
      glVertex3f(nx * r2, h2, nz * r2);
    }
    glEnd();
  }
}

// Custom leaf cluster using intersecting quads for volume
void drawCustomLeafCluster(float size, int density, float r, float g, float b) {
  glColor3f(r, g, b);
  glBegin(GL_QUADS);
  for (int i = 0; i < density; i++) {
    float ax = (float)((i * 37) % 360) * 3.14159f / 180.0f;
    float ay = (float)((i * 73) % 360) * 3.14159f / 180.0f;
    float az = (float)((i * 101) % 360) * 3.14159f / 180.0f;

    float sx = sinf(ax) * size;
    float cx = cosf(ax) * size;
    float sy = sinf(ay) * size;
    float cy = cosf(ay) * size;
    float sz = sinf(az) * size;
    float cz = cosf(az) * size;

    glNormal3f(sx, sy, sz);
    glVertex3f(sx + cx, sy + cy, sz + cz);
    glVertex3f(sx - cx, sy + cy, sz - cz);
    glVertex3f(sx - cx, sy - cy, sz - cz);
    glVertex3f(sx + cx, sy - cy, sz + cz);

    // Second intersecting quad
    glNormal3f(cx, cy, cz);
    glVertex3f(cx + sx, cy + sy, cz + sz);
    glVertex3f(cx - sx, cy + sy, cz - sz);
    glVertex3f(cx - sx, cy - sy, cz - sz);
    glVertex3f(cx + sx, cy - sy, cz + sz);
  }
  glEnd();
}

// ---------------------------------------------------------
// 1. OAK TREE (Massive branching, dense leaves)
// ---------------------------------------------------------
void drawOakBranch(float length, float radius, int depth, int maxDepth,
                   int seed) {
  if (depth > maxDepth)
    return;

  glPushMatrix();
  glColor3f(0.25f - depth * 0.02f, 0.20f - depth * 0.015f, 0.15f);

  // Slight sway for deeper branches
  if (depth > 1) {
    float sway = sinf((float)seed * 0.1f) * 5.0f * depth;
    glRotatef(sway, 1, 0, 1);
  }

  drawCustomBranch(radius, radius * 0.7f, length, 8 - depth, 3);
  glTranslatef(0.0f, length, 0.0f);

  if (depth >= maxDepth - 1) {
    float lr = 0.15f + ((seed % 10) * 0.01f);
    float lg = 0.35f + ((seed % 20) * 0.01f);
    float lb = 0.10f + ((seed % 10) * 0.01f);
    drawCustomLeafCluster(length * 1.5f, 15, lr, lg, lb);
  }

  if (depth < maxDepth) {
    int branches = 2 + (seed % 3); // 2 to 4 branches
    for (int i = 0; i < branches; i++) {
      int subSeed = seed * 17 + i * 31;
      glPushMatrix();
      float bend = 25.0f + (subSeed % 30);
      float rot = (360.0f / branches) * i + (subSeed % 60 - 30);
      glRotatef(rot, 0, 1, 0);
      glRotatef(bend, 1, 0, 0);
      drawOakBranch(length * 0.75f, radius * 0.65f, depth + 1, maxDepth,
                    subSeed);
      glPopMatrix();
    }
  }
  glPopMatrix();
}

void drawProceduralOak(float x, float z) {
  glPushMatrix();
  glTranslatef(x, 0.0f, z);
  int seed = (int)(fabs(x) * 100 + fabs(z) * 100);
  drawOakBranch(1.8f, 0.25f, 0, 4, seed);

  // Add exposed roots
  glColor3f(0.25f, 0.20f, 0.15f);
  for (int i = 0; i < 4; i++) {
    glPushMatrix();
    glRotatef(i * 90.0f + (seed % 45), 0, 1, 0);
    glRotatef(60.0f, 1, 0, 0);
    drawCustomBranch(0.15f, 0.05f, 0.8f, 6, 2);
    glPopMatrix();
  }
  glPopMatrix();
}

// ---------------------------------------------------------
// 2. WEEPING WILLOW (Drooping vines, wide canopy)
// ---------------------------------------------------------
void drawWillowVine(float length, int segments, int seed) {
  glPushMatrix();
  glColor3f(0.4f, 0.6f, 0.2f);
  float segLen = length / segments;
  float sway = sinf((float)seed * 0.05f) * 5.0f;
  for (int i = 0; i < segments; i++) {
    glRotatef(sway + (seed % 5), 1, 0, 0);
    drawCustomBranch(0.02f, 0.01f, segLen, 4, 1);
    glTranslatef(0.0f, segLen, 0.0f);
    // Leaf on vine
    if (i % 2 == 0)
      drawCustomLeafCluster(0.15f, 2, 0.45f, 0.65f, 0.25f);
  }
  glPopMatrix();
}

void drawProceduralWillow(float x, float z) {
  glPushMatrix();
  glTranslatef(x, 0.0f, z);
  int seed = (int)(fabs(x) * 100 + fabs(z) * 100);

  // Short thick trunk
  glColor3f(0.3f, 0.25f, 0.2f);
  drawCustomBranch(0.4f, 0.35f, 1.5f, 12, 3);
  glTranslatef(0.0f, 1.5f, 0.0f);

  // Wide dome structure
  int mainBranches = 6;
  for (int i = 0; i < mainBranches; i++) {
    glPushMatrix();
    glRotatef((360.0f / mainBranches) * i, 0, 1, 0);
    glRotatef(45.0f, 1, 0, 0);
    glColor3f(0.3f, 0.25f, 0.2f);
    drawCustomBranch(0.2f, 0.1f, 1.8f, 6, 2);
    glTranslatef(0.0f, 1.8f, 0.0f);

    // Spawn vines
    for (int v = 0; v < 8; v++) {
      glPushMatrix();
      glRotatef(v * 45.0f, 0, 1, 0);
      glRotatef(120.0f, 1, 0, 0); // pointing downwards
      drawWillowVine(2.5f + (seed % 10) * 0.1f, 8, seed * v);
      glPopMatrix();
    }
    glPopMatrix();
  }
  glPopMatrix();
}

// ---------------------------------------------------------
// 3. DETAILED PINE (Tiered needles)
// ---------------------------------------------------------
void drawProceduralPine(float x, float z) {
  glPushMatrix();
  glTranslatef(x, 0.0f, z);
  int seed = (int)(fabs(x) * 100 + fabs(z) * 100);

  float height = 4.0f + (seed % 20) * 0.1f;
  glColor3f(0.2f, 0.15f, 0.1f);
  drawCustomBranch(0.2f, 0.05f, height, 8, 4);

  // Tiers of needles
  int tiers = 8 + (seed % 4);
  float tierSpacing = height / (tiers + 1);
  for (int i = 1; i <= tiers; i++) {
    glPushMatrix();
    glTranslatef(0.0f, i * tierSpacing, 0.0f);
    float radius = (height - i * tierSpacing) * 0.6f;
    int branches = 6 + i;
    for (int b = 0; b < branches; b++) {
      glPushMatrix();
      glRotatef((360.0f / branches) * b + (i * 15.0f), 0, 1, 0);
      glRotatef(80.0f + (seed % 10), 1, 0, 0);
      glColor3f(0.1f, 0.3f, 0.15f);
      drawCustomBranch(0.05f, 0.01f, radius, 4, 1);
      glTranslatef(0.0f, radius * 0.5f, 0.0f);
      drawCustomLeafCluster(radius * 0.6f, 6, 0.1f, 0.3f, 0.15f);
      glPopMatrix();
    }
    glPopMatrix();
  }
  glPopMatrix();
}

// ---------------------------------------------------------
// 4. JACARANDA (Purple blooms, twisting trunk)
// ---------------------------------------------------------
void drawJacarandaBranch(float length, float radius, int depth, int maxDepth,
                         int seed) {
  if (depth > maxDepth)
    return;

  glPushMatrix();
  glColor3f(0.4f, 0.35f, 0.3f);

  // Twisting effect
  glRotatef(15.0f * sinf(length), 0, 1, 0);
  drawCustomBranch(radius, radius * 0.6f, length, 6, 2);
  glTranslatef(0.0f, length, 0.0f);

  if (depth >= maxDepth - 1) {
    // Bright purple floral clusters
    drawCustomLeafCluster(length * 1.8f, 20, 0.6f, 0.2f, 0.8f);
    // Occasional fallen petals
    if (seed % 3 == 0) {
      glPushMatrix();
      glTranslatef(0, -length * 2.0f, 0);
      drawCustomLeafCluster(0.3f, 3, 0.6f, 0.2f, 0.8f);
      glPopMatrix();
    }
  }

  if (depth < maxDepth) {
    int branches = 2 + (seed % 2);
    for (int i = 0; i < branches; i++) {
      int subSeed = seed * 11 + i * 23;
      glPushMatrix();
      float bend = 30.0f + (subSeed % 20);
      float rot = (360.0f / branches) * i + (subSeed % 90 - 45);
      glRotatef(rot, 0, 1, 0);
      glRotatef(bend, 1, 0, 0);
      drawJacarandaBranch(length * 0.8f, radius * 0.6f, depth + 1, maxDepth,
                          subSeed);
      glPopMatrix();
    }
  }
  glPopMatrix();
}

void drawProceduralJacaranda(float x, float z) {
  glPushMatrix();
  glTranslatef(x, 0.0f, z);
  int seed = (int)(fabs(x) * 100 + fabs(z) * 100);
  drawJacarandaBranch(1.5f, 0.2f, 0, 3, seed);
  glPopMatrix();
}

// ---------------------------------------------------------
// 5. AFRICAN ACACIA (Umbrella shaped canopy)
// ---------------------------------------------------------
void drawAcaciaBranch(float length, float radius, int depth, int maxDepth,
                      int seed) {
  if (depth > maxDepth)
    return;
  glPushMatrix();
  glColor3f(0.5f, 0.45f, 0.4f);

  // Acacia branches bend sharply horizontally
  if (depth > 0) {
    glRotatef(50.0f + (seed % 20), 1, 0, 0);
  }

  drawCustomBranch(radius, radius * 0.7f, length, 6, 2);
  glTranslatef(0.0f, length, 0.0f);

  if (depth == maxDepth) {
    // Flat top canopy
    glPushMatrix();
    glRotatef(-90.0f, 1, 0, 0); // flatten
    drawCustomLeafCluster(length * 2.5f, 12, 0.3f, 0.5f, 0.2f);
    glPopMatrix();
  }

  if (depth < maxDepth) {
    int branches = (depth == 0) ? 4 : 2;
    for (int i = 0; i < branches; i++) {
      int subSeed = seed * 13 + i * 19;
      glPushMatrix();
      float rot = (360.0f / branches) * i + (subSeed % 40 - 20);
      glRotatef(rot, 0, 1, 0);
      drawAcaciaBranch(length * 0.9f, radius * 0.6f, depth + 1, maxDepth,
                       subSeed);
      glPopMatrix();
    }
  }
  glPopMatrix();
}

void drawProceduralAcacia(float x, float z) {
  glPushMatrix();
  glTranslatef(x, 0.0f, z);
  int seed = (int)(fabs(x) * 100 + fabs(z) * 100);
  drawAcaciaBranch(1.2f, 0.3f, 0, 3, seed);
  glPopMatrix();
}
void drawTree(float x, float z, int type) {
  if (type == 2) {
    drawProceduralOak(x, z);
    return;
  }
  if (type == 3) {
    drawProceduralWillow(x, z);
    return;
  }
  if (type == 4) {
    drawProceduralPine(x, z);
    return;
  }
  if (type == 5) {
    drawProceduralJacaranda(x, z);
    return;
  }
  if (type == 6) {
    drawProceduralAcacia(x, z);
    return;
  }

  glPushMatrix();
  glTranslatef(x, 0.0f, z);

  // Trunk (100% Solid Rounded Cylinder)
  glPushMatrix();
  glColor3f(0.35f, 0.25f, 0.15f);
  glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Rotate to stand vertically
  GLUquadric* quad = gluNewQuadric();
  gluQuadricNormals(quad, GLU_SMOOTH);
  gluCylinder(quad, 0.12f, 0.09f, 1.5f, 8, 1); // Solid cylindrical trunk
  gluDeleteQuadric(quad);
  glPopMatrix();

  if (type == 0) { // Pine / Evergreen (Cone shape)
    glColor3f(0.08f, 0.25f, 0.05f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.8, 2.5, 8, 4);
    glPopMatrix();
  } else { // Leafy Tree (Sphere-ish)
    glColor3f(0.15f, 0.45f, 0.1f);
    glPushMatrix();
    glTranslatef(0, 2.2, 0);
    glutSolidSphere(1.0, 8, 8);
    glPopMatrix();
  }
  glPopMatrix();
}

void drawFanCobblestonePath(float x1, float z1, float x2, float z2) {
  float fanSpacing = 1.6f;
  float stoneSize = 0.14f;
  float gap = 0.02f;
  float step = stoneSize + gap;

  // Tile "fan units" along the area
  for (float fx = x1 - fanSpacing; fx < x2 + fanSpacing; fx += fanSpacing) {
    for (float fz = z1 - fanSpacing; fz < z2 + fanSpacing; fz += fanSpacing) {
      // Local fan center
      float cx = fx;
      float cz = fz;

      // Pseudo-random seed for this fan
      int fSeed = (int)((fx + 100.0f) * 17.0f + (fz + 100.0f) * 23.0f);

      // concentric arcs
      for (float r = 0.15f; r < fanSpacing * 0.85f; r += step) {
        float arcAngle = 140.0f; // degrees
        float startAngle = -arcAngle / 2.0f;
        float arcLen = (arcAngle / 180.0f) * 3.14159f * r;
        int numStones = (int)(arcLen / step);
        if (numStones < 1)
          numStones = 1;

        for (int i = 0; i <= numStones; i++) {
          float angleDeg = startAngle + (float)i / numStones * arcAngle;
          float angleRad = angleDeg * 3.14159f / 180.0f;

          float sx = cx + r * sinf(angleRad);
          float sz = cz + r * cosf(angleRad);

          // Clipping to path bounds
          if (sx < x1 || sx > x2 || sz < z1 || sz > z2)
            continue;

          // Exclude zones where drainage grates are placed
          if (sz >= 9.0f && sz <= 10.0f)
            continue; // Deep ditch grate
          if (sz >= 11.65f && sz <= 12.0f)
            continue; // Road entrance grate

          // Stone Color logic (Mostly grey, some scattered light ones)
          int sSeed = fSeed + (int)(r * 100) + i;
          bool isLight = (sSeed % 13 == 0);
          float rCol, gCol, bCol;
          if (isLight) {
            rCol = 0.88f;
            gCol = 0.85f;
            bCol = 0.80f; // Cream/White
          } else {
            float v = (float)(sSeed % 20) / 100.0f;
            rCol = 0.42f + v;
            gCol = 0.42f + v;
            bCol = 0.44f + v; // Grey tones
          }

          // Slight jitter
          float jx = (float)(sSeed % 5) * 0.005f - 0.01f;
          float jz = (float)(sSeed % 7) * 0.005f - 0.01f;
          float jy = (float)(sSeed % 3) * 0.005f;

          solidBox(sx + jx, -0.01f, sz + jz, sx + stoneSize + jx, 0.02f + jy,
                   sz + stoneSize + jz, rCol, gCol, bCol, rCol * 0.85f,
                   gCol * 0.85f, bCol * 0.85f, rCol * 1.05f, gCol * 1.05f,
                   bCol * 1.05f);
        }
      }
    }
  }
}

void drawBush(float x, float z) {
  glPushMatrix();
  glTranslatef(x, 0.0f, z);
  glColor3f(0.6f, 0.72f, 0.15f); // Bright yellowish green

  // Cluster of 5 spheres for a bushy look
  float offX[5] = {0.0f, 0.15f, -0.15f, 0.12f, -0.12f};
  float offZ[5] = {0.0f, 0.12f, 0.12f, -0.15f, -0.15f};
  float offY[5] = {0.25f, 0.20f, 0.20f, 0.18f, 0.18f};

  for (int i = 0; i < 5; i++) {
    glPushMatrix();
    glTranslatef(offX[i], offY[i], offZ[i]);
    glutSolidSphere(0.25f, 6, 6);
    glPopMatrix();
  }
  glPopMatrix();
}

void drawLongGrass(float x, float z) {
  glLineWidth(1.0f);
  glColor3f(0.35f, 0.55f, 0.25f);
  glBegin(GL_LINES);
  // Draw 3-4 simple blades of grass
  glVertex3f(x, 0.0f, z);
  glVertex3f(x - 0.05f, 0.2f, z + 0.05f);
  glVertex3f(x, 0.0f, z);
  glVertex3f(x + 0.05f, 0.25f, z - 0.02f);
  glVertex3f(x, 0.0f, z);
  glVertex3f(x, 0.18f, z + 0.08f);
  glEnd();
}

void drawDitch(float xCenter, float z1, float z2) {
  float w = 0.5f;                 // ditch width
  float d = 0.25f;                // depth
  glColor3f(0.42f, 0.48f, 0.38f); // Mossy concrete grey-green
  glBegin(GL_QUADS);
  // Left slope
  glNormal3f(0.447f, 0.894f, 0.0f); // Approximate normal for slanted face
  glVertex3f(xCenter - w / 2, 0.0f, z1);
  glVertex3f(xCenter, -d, z1);
  glVertex3f(xCenter, -d, z2);
  glVertex3f(xCenter - w / 2, 0.0f, z2);
  // Right slope
  glNormal3f(-0.447f, 0.894f, 0.0f);
  glVertex3f(xCenter, -d, z1);
  glVertex3f(xCenter + w / 2, 0.0f, z1);
  glVertex3f(xCenter + w / 2, 0.0f, z2);
  glVertex3f(xCenter, -d, z2);
  glEnd();
}

void drawDitchHorizontal(float zCenter, float x1, float x2, float w = 0.5f,
                         float d = 0.25f) {
  // Draw the deep hole (V-shape ditch)
  glColor3f(0.55f, 0.55f, 0.52f); // Concrete color to match the border
  glBegin(GL_QUADS);
  // Back slope
  glNormal3f(0.0f, 0.894f, 0.447f);
  glVertex3f(x1, 0.0f, zCenter - w / 2);
  glVertex3f(x1, -d, zCenter);
  glVertex3f(x2, -d, zCenter);
  glVertex3f(x2, 0.0f, zCenter - w / 2);
  // Front slope
  glNormal3f(0.0f, 0.894f, -0.447f);
  glVertex3f(x1, -d, zCenter);
  glVertex3f(x1, 0.0f, zCenter + w / 2);
  glVertex3f(x2, 0.0f, zCenter + w / 2);
  glVertex3f(x2, -d, zCenter);
  glEnd();

  // End caps to close the holes (Triangles)
  glBegin(GL_TRIANGLES);
  // Left cap (at x1)
  glNormal3f(-1.0f, 0.0f, 0.0f);
  glVertex3f(x1, 0.0f, zCenter - w / 2);
  glVertex3f(x1, 0.0f, zCenter + w / 2);
  glVertex3f(x1, -d, zCenter);
  // Right cap (at x2)
  glNormal3f(1.0f, 0.0f, 0.0f);
  glVertex3f(x2, 0.0f, zCenter - w / 2);
  glVertex3f(x2, -d, zCenter);
  glVertex3f(x2, 0.0f, zCenter + w / 2);
  glEnd();
}

void drawStreetLight(float x, float z) {
  glPushMatrix();
  glTranslatef(x, 0.0f, z);

  // Main Pole
  glColor3f(0.55f, 0.58f, 0.62f);
  float pw = 0.08f;
  solidBox(-pw, 0, -pw, pw, 4.5f, pw, 0.5f, 0.53f, 0.57f, 0.45f, 0.48f, 0.52f,
           0.6f, 0.63f, 0.67f);

  // Arched Arm
  solidBox(-pw, 4.2f, -pw, 0.8f, 4.4f, pw, 0.55f, 0.58f, 0.62f, 0.5f, 0.53f,
           0.57f, 0.65f, 0.68f, 0.72f);

  // Lamp Head
  solidBox(0.4f, 4.15f, -0.15f, 1.0f, 4.35f, 0.15f, 0.4f, 0.4f, 0.4f, 0.35f,
           0.35f, 0.35f, 0.5f, 0.5f, 0.5f);

  // Bulb/Glow at night
  if (isNight) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.6f);
    glPushMatrix();
    glTranslatef(0.7f, 4.14f, 0.0f);
    glutSolidSphere(0.12f, 12, 12);
    glPopMatrix();
    glEnable(GL_LIGHTING);
  }

  glPopMatrix();
}

void drawCurbSegment(float x1, float z1, float x2, float z2, bool isWhite) {
  float r = isWhite ? 0.95f : 0.15f;
  float g = isWhite ? 0.95f : 0.15f;
  float b = isWhite ? 0.95f : 0.15f;
  float dx = x2 - x1;
  float dz = z2 - z1;
  float len = sqrtf(dx * dx + dz * dz);
  float angle = atan2f(dz, dx) * 180.0f / 3.14159f;

  glPushMatrix();
  glTranslatef(x1, 0.0f, z1);
  glRotatef(-angle, 0, 1, 0);
  solidBox(0, 0, 0, len, 0.18f, 0.35f, r, g, b, r * 0.8f, g * 0.8f, b * 0.8f,
           r * 1.1f, g * 1.1f, b * 1.1f);
  glPopMatrix();
}

void drawRoundedCorner(float cx, float cz, float radius, float startAngle,
                       float sweepAngle, bool startWhite) {
  int segments = 8;
  float step = sweepAngle / segments;
  for (int i = 0; i < segments; i++) {
    float a1 = (startAngle + i * step) * 3.14159f / 180.0f;
    float a2 = (startAngle + (i + 1) * step) * 3.14159f / 180.0f;
    float x1 = cx + radius * cosf(a1);
    float z1 = cz + radius * sinf(a1);
    float x2 = cx + radius * cosf(a2);
    float z2 = cz + radius * sinf(a2);
    bool isWhite = (i < segments / 2) ? startWhite : !startWhite;
    drawCurbSegment(x1, z1, x2, z2, isWhite);
  }
}

void drawPathEntranceGrate(float x1, float x2, float z1, float z2) {
  // Draw solid black base to simulate deep hole and cover the grass underneath
  glColor3f(0.05f, 0.05f, 0.05f); // Very dark/black
  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);
  glVertex3f(x1, -0.018f, z1);
  glVertex3f(x2, -0.018f, z1);
  glVertex3f(x2, -0.018f, z2);
  glVertex3f(x1, -0.018f, z2);
  glEnd();

  float slabW = 0.35f;
  float gap = 0.08f;
  glColor3f(0.6f, 0.6f, 0.58f); // Concrete grey
  for (float x = x1; x < x2; x += slabW + gap) {
    // Draw concrete slab
    solidBox(x, -0.01f, z1, x + slabW, 0.04f, z2, 0.65f, 0.65f, 0.62f, 0.55f,
             0.55f, 0.52f, 0.7f, 0.7f, 0.68f);

    // Draw dark gap/shadow under it
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex3f(x + slabW, -0.015f, z1);
    glVertex3f(x + slabW + gap, -0.015f, z1);
    glVertex3f(x + slabW + gap, -0.015f, z2);
    glVertex3f(x + slabW, -0.015f, z2);
    glEnd();
  }
}

void drawPatternedSidewalk(float x1, float z1, float x2, float z2) {
  float tw = 0.4f, td = 0.4f;
  float gap = 0.02f;
  for (float x = x1; x < x2; x += tw + gap) {
    // Skip sidewalk where paths are
    if ((x > -9.2f && x < -5.8f) || (x > 7.55f && x < 10.95f) ||
        (x > -25.95f && x < -22.55f))
      continue;

    for (float z = z1; z < z2; z += td + gap) {
      int ix = (int)((x - x1 + 0.01f) / (tw + gap));
      int iz = (int)((z - z1 + 0.01f) / (td + gap));
      // Middle column (index 1) is grey/white, outer columns (0 and 2) are red
      bool isRed = (iz != 1);
      float r, g, b;
      if (isRed) {
        r = 0.72f;
        g = 0.35f;
        b = 0.30f;
      } else {
        r = 0.75f;
        g = 0.75f;
        b = 0.72f;
      }
      solidBox(x, -0.01f, z, x + tw, 0.03f, z + td, r, g, b, r * 0.8f, g * 0.8f,
               b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
    }
  }
}

void drawDetailedDoor(float x, float y, float z, float openAngle = 0.0f) {
  float w = 0.6f;
  float h = 0.9f;         // Main door height
  float transomH = 0.25f; // Window above door

  // 1. Transom Window (Above door)
  glColor3f(0.05f, 0.05f, 0.12f); // Dark glass
  glBegin(GL_QUADS);
  glVertex3f(x - w / 2, y + h, z);
  glVertex3f(x + w / 2, y + h, z);
  glVertex3f(x + w / 2, y + h + transomH, z);
  glVertex3f(x - w / 2, y + h + transomH, z);
  glEnd();

  // 6. Frame
  glColor3f(0.75f, 0.72f, 0.68f);
  glLineWidth(2.0f);
  glBegin(GL_LINE_LOOP);
  glVertex3f(x - w / 2, y, z + 0.001f);
  glVertex3f(x + w / 2, y, z + 0.001f);
  glVertex3f(x + w / 2, y + h + transomH, z + 0.001f);
  glVertex3f(x - w / 2, y + h + transomH, z + 0.001f);
  glEnd();

  // 0. Dark Interior Hole (Revealed when door opens)
  // Using a thin box instead of a quad to ensure it masks the wall and has
  // depth
  solidBox(x - w / 2, y, z - 0.01f, x + w / 2, y + h, z - 0.001f, 0.0f, 0.0f,
           0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

  glPushMatrix();
  // Hinge on the left side
  glTranslatef(x - w / 2, y, z);
  glRotatef(openAngle, 0, 1, 0);
  glTranslatef(-(x - w / 2), -y, -z);

  // 2. Door Surface (Light Tan)
  glColor3f(0.92f, 0.88f, 0.82f);
  glBegin(GL_QUADS);
  glVertex3f(x - w / 2, y, z);
  glVertex3f(x + w / 2, y, z);
  glVertex3f(x + w / 2, y + h, z);
  glVertex3f(x - w / 2, y + h, z);
  glEnd();

  // 3. Vertical Ridges (Thin lines)
  glColor3f(0.75f, 0.72f, 0.68f);
  glLineWidth(1.0f);
  glBegin(GL_LINES);
  for (float rx = -w / 2 + 0.06f; rx <= w / 2 - 0.06f; rx += 0.06f) {
    glVertex3f(x + rx, y, z + 0.005f);
    glVertex3f(x + rx, y + h, z + 0.005f);
  }
  glEnd();

  // 4. Horizontal Middle Bar
  glColor3f(0.88f, 0.84f, 0.80f);
  glBegin(GL_QUADS);
  glVertex3f(x - w / 2, y + h / 2 - 0.02f, z + 0.008f);
  glVertex3f(x + w / 2, y + h / 2 - 0.02f, z + 0.008f);
  glVertex3f(x + w / 2, y + h / 2 + 0.02f, z + 0.008f);
  glVertex3f(x - w / 2, y + h / 2 + 0.02f, z + 0.008f);
  glEnd();

  // 5. Handle/Lock Detail
  glColor3f(0.35f, 0.35f, 0.35f);
  glPushMatrix();
  glTranslatef(x + w / 2 - 0.1f, y + h / 2, z + 0.015f);
  glutSolidCube(0.04f);
  glPopMatrix();

  glPopMatrix(); // End of door hinge
}

void drawStudent(float x, float y, float z, float angle, float walkPhase = 0.0f,
                 bool isSeated = false, int variant = 0,
                 bool isTeaching = false) {
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
    if (sinf(walkPhase * 0.1f) > 0.6f)
      teacherAngle += 180.0f;
  }

  // Body bobbing
  float bob =
      (isSeated || isTeaching) ? 0.0f : fabsf(sinf(walkPhase * 2.0f)) * 0.05f;
  glTranslatef(tx, y + bob, z);
  glRotatef(teacherAngle, 0, 1, 0);

  // Teachers are slightly taller (15% more)
  bool isTeacher = (isTeaching || variant == 99);
  float scale = isTeacher ? 0.52f : 0.45f;
  glScalef(scale, scale, scale);

  // 1. COLORS
  float shirtColors[6][3] = {{0.2f, 0.3f, 0.8f}, {0.8f, 0.2f, 0.2f},
                             {0.2f, 0.7f, 0.3f}, {0.9f, 0.8f, 0.2f},
                             {0.6f, 0.2f, 0.7f}, {0.2f, 0.8f, 0.8f}};
  float teacherSuit[3] = {0.05f, 0.05f, 0.05f}; // Pure black formal suit

  float hairColors[4][3] = {{0.05f, 0.05f, 0.05f},
                            {0.35f, 0.2f, 0.1f},
                            {0.8f, 0.7f, 0.3f},
                            {0.5f, 0.5f, 0.5f}};
  int sCol = variant % 6;
  int hCol = variant % 4;

  // 2. TORSO
  if (isTeacher)
    glColor3fv(teacherSuit);
  else
    glColor3fv(shirtColors[sCol]);

  solidBox(-0.22f, 0.45f, -0.12f, 0.22f, 1.05f, 0.12f,
           isTeacher ? teacherSuit[0] : shirtColors[sCol][0],
           isTeacher ? teacherSuit[1] : shirtColors[sCol][1],
           isTeacher ? teacherSuit[2] : shirtColors[sCol][2], 0.1f, 0.1f, 0.12f, 0.2f,
           0.2f, 0.25f);

  // 2.5 PANTS / PELVIS
  glColor3f(0.1f, 0.12f, 0.15f);
  solidBox(-0.22f, 0.35f, -0.12f, 0.22f, 0.45f, 0.12f, 0.1f, 0.12f, 0.15f,
           0.05f, 0.06f, 0.08f, 0.15f, 0.18f, 0.2f);

  // 3. HEAD
  glPushMatrix();
  glTranslatef(0.0f, 1.25f, 0.0f);
  glColor3f(0.9f, 0.7f, 0.6f);
  glutSolidSphere(0.22f, 20, 16);

  glPushMatrix(); // Hair
  glTranslatef(0.0f, 0.08f, 0.0f);
  glScalef(1.05f, 0.7f, 1.05f);
  if (isTeacher)
      glColor3f(0.05f, 0.05f, 0.05f); // Pure black hair for teacher
  else
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

    if (isSeated)
      glRotatef(90, 1, 0, 0);
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

    if (isTeaching)
      glColor3fv(teacherSuit);
    else
      glColor3fv(shirtColors[sCol]);

    glRotatef(90, 1, 0, 0);
    gluCylinder(qobj, 0.06, 0.05, 0.35, 16, 2);

    glTranslatef(0, 0, 0.35f); // Elbow
    glutSolidSphere(0.05f, 10, 10);

    glColor3f(0.9f, 0.7f, 0.6f);
    if (isSeated)
      glRotatef(-60, 1, 0, 0);
    else if (isTeaching)
      glRotatef(-40, 1, 0, 0); // Professional bend

    gluCylinder(qobj, 0.05, 0.04, 0.35, 16, 2);
    glPopMatrix();
  }

  glPopMatrix();
}

void drawStudents() {
  // If it's night time, the campus is deserted!
  if (isNight) return;

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
    float studentFacing1 =
        atan2f(studentDirX1, studentDirZ1) * 180.0f / 3.14159f;

    drawStudent(studentPosX1, studentPosY1, studentPosZ1, studentFacing1,
                (studentWalkAngle + phaseOffset) * 0.1f, false, i);

    // Ground floor students pacing near the ground floor doors
    // If we are transitioning, student 0 detaches and becomes the entering
    // student
    if (currentMode != MODE_EXTERIOR && i == 0) {
      drawStudent(enteringStudentX, 0.0f, enteringStudentZ, 180.0f,
                  transitionProgress * 20.0f, false, i + 10); // facing inside
    } else {
      // Give them a slightly different speed multiplier and phase
      float studentRad2 =
          (studentWalkAngle * 0.8f + phaseOffset + 60.0f) * 3.14159f / 180.0f;
      float studentPosX2 = sinf(studentRad2) * 3.5f;
      float studentPosZ2 =
          1.35f + cosf(studentRad2) *
                      0.15f;     // Ground floor walkway (between 0.7 and 1.8)
      float studentPosY2 = 0.0f; // Ground floor height

      float studentDirX2 = cosf(studentRad2) * 3.5f;
      float studentDirZ2 = -sinf(studentRad2) * 0.15f;
      float studentFacing2 =
          atan2f(studentDirX2, studentDirZ2) * 180.0f / 3.14159f;

      drawStudent(studentPosX2, studentPosY2, studentPosZ2, studentFacing2,
                  (studentWalkAngle * 0.8f + phaseOffset + 60.0f) * 0.1f, false,
                  i + 20);
    }
  }
}

void drawClassroomInterior() {
  // Draw the hollowed-out room on the ground floor behind the middle door.
  // Narrower room to match reference photo
  float rX1 = -3.0f, rX2 = 3.0f;
  float rZ1 = -7.5f, rZ2 = 0.65f;
  float rY1 = 0.01f, rY2 = 1.5f; // Realistic ceiling height

  // Floor (Light Tile pattern)
  glColor3f(0.8f, 0.8f, 0.8f);
  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);
  glVertex3f(rX1, rY1, rZ2);
  glVertex3f(rX2, rY1, rZ2);
  glVertex3f(rX2, rY1, rZ1);
  glVertex3f(rX1, rY1, rZ1);
  glEnd();

  // Ceiling (Light)
  glColor3f(0.9f, 0.9f, 0.9f);
  glBegin(GL_QUADS);
  glNormal3f(0, -1, 0);
  glVertex3f(rX1, rY2, rZ1);
  glVertex3f(rX2, rY2, rZ1);
  glVertex3f(rX2, rY2, rZ2);
  glVertex3f(rX1, rY2, rZ2);
  glEnd();

  // Walls - Universal two-tone logic
  float walls[][6] = {
      {rX1, rY1, rZ1, rX1, rY2, rZ2}, // Left
      {rX2, rY1, rZ2, rX2, rY2, rZ1}, // Right
      {rX1, rY1, rZ1, rX2, rY2, rZ1}, // Front
      {rX2, rY1, rZ2, rX1, rY2, rZ2}  // Back
  };
  float normals[][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}};

  for (int i = 0; i < 4; i++) {
    glBegin(GL_QUADS);
    glNormal3fv(normals[i]);
    // Bottom Mustard/Ochre Band
    glColor3f(0.78f, 0.62f, 0.35f);
    glVertex3f(walls[i][0], walls[i][1], walls[i][2]);
    glVertex3f(walls[i][3], walls[i][1], walls[i][5]);
    glVertex3f(walls[i][3], 0.5f, walls[i][5]);
    glVertex3f(walls[i][0], 0.5f, walls[i][2]);
    // Top White Band
    glColor3f(0.92f, 0.92f, 0.92f);
    glVertex3f(walls[i][0], 0.5f, walls[i][2]);
    glVertex3f(walls[i][3], 0.5f, walls[i][5]);
    glVertex3f(walls[i][3], walls[i][4], walls[i][5]);
    glVertex3f(walls[i][0], walls[i][4], walls[i][2]);
    glEnd();
  }

  // Ceiling Beam (Front)
  glColor3f(0.85f, 0.85f, 0.85f);
  solidBox(rX1, rY2 - 0.2f, rZ1 + 1.2f, rX2, rY2, rZ1 + 1.5f, 0.85f, 0.85f,
           0.85f, 0.8f, 0.8f, 0.8f, 0.9f, 0.9f, 0.9f);

  // Columns supporting the beam
  // Right
  solidBox(rX2 - 0.3f, rY1, rZ1 + 1.2f, rX2, 0.5f, rZ1 + 1.5f, 0.78f, 0.62f,
           0.35f, 0.73f, 0.57f, 0.3f, 0.83f, 0.67f, 0.4f);
  solidBox(rX2 - 0.3f, 0.5f, rZ1 + 1.2f, rX2, rY2, rZ1 + 1.5f, 0.92f, 0.92f,
           0.92f, 0.87f, 0.87f, 0.87f, 0.97f, 0.97f, 0.97f);
  // Left
  solidBox(rX1, rY1, rZ1 + 1.2f, rX1 + 0.3f, 0.5f, rZ1 + 1.5f, 0.78f, 0.62f,
           0.35f, 0.73f, 0.57f, 0.3f, 0.83f, 0.67f, 0.4f);
  solidBox(rX1, 0.5f, rZ1 + 1.2f, rX1 + 0.3f, rY2, rZ1 + 1.5f, 0.92f, 0.92f,
           0.92f, 0.87f, 0.87f, 0.87f, 0.97f, 0.97f, 0.97f);

  // Front Wall Windows (Only on the right side, behind TV/Podium)
  float fwX1 = 0.5f, fwX2 = rX2 - 0.1f;
  float fwY1 = 0.85f, fwY2 = 1.25f;
  glColor3f(0.8f, 0.9f, 1.0f); // Bright daylight glass
  glDisable(GL_LIGHTING);
  glBegin(GL_QUADS);
  glVertex3f(fwX1, fwY1, rZ1 + 0.005f);
  glVertex3f(fwX2, fwY1, rZ1 + 0.005f);
  glVertex3f(fwX2, fwY2, rZ1 + 0.005f);
  glVertex3f(fwX1, fwY2, rZ1 + 0.005f);
  glEnd();
  glEnable(GL_LIGHTING);
  // Window Frames
  glColor3f(0.7f, 0.7f, 0.75f); // Metallic frame
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  glVertex3f(fwX1, fwY1, rZ1 + 0.006f);
  glVertex3f(fwX2, fwY1, rZ1 + 0.006f);
  glVertex3f(fwX1, fwY2, rZ1 + 0.006f);
  glVertex3f(fwX2, fwY2, rZ1 + 0.006f);
  glVertex3f(fwX1, (fwY1 + fwY2) * 0.5f, rZ1 + 0.006f);
  glVertex3f(fwX2, (fwY1 + fwY2) * 0.5f, rZ1 + 0.006f); // 1 horizontal line
  for (float wx = fwX1; wx <= fwX2 + 0.01f;
       wx += (fwX2 - fwX1) / 4.0f) { // 4 panes
    glVertex3f(wx, fwY1, rZ1 + 0.006f);
    glVertex3f(wx, fwY2, rZ1 + 0.006f);
  }
  glEnd();

  // Back Wall Window (Large 4-pane window)
  float wY1 = 0.5f, wY2 = 1.3f;  // Taller, starts exactly at mustard line
  float wX1 = -2.8f, wX2 = 2.2f; // Offset to leave wall space on the left side
                                 // (from photo's perspective)
  glDisable(GL_LIGHTING);        // Bright daylight
  glColor3f(0.85f, 0.95f, 1.0f);
  glBegin(GL_QUADS);
  glVertex3f(wX1, wY1, rZ2 - 0.005f);
  glVertex3f(wX2, wY1, rZ2 - 0.005f);
  glVertex3f(wX2, wY2, rZ2 - 0.005f);
  glVertex3f(wX1, wY2, rZ2 - 0.005f);
  glEnd();
  glEnable(GL_LIGHTING);
  // Back Window Frames
  glColor3f(0.7f, 0.7f, 0.75f); // Metallic frame

  // Thin horizontal lines (2 lines dividing window into 3 vertical segments)
  glLineWidth(2.0f);
  glBegin(GL_LINES);
  // Top and bottom borders
  glVertex3f(wX1, wY1, rZ2 - 0.006f);
  glVertex3f(wX2, wY1, rZ2 - 0.006f);
  glVertex3f(wX1, wY2, rZ2 - 0.006f);
  glVertex3f(wX2, wY2, rZ2 - 0.006f);
  // 2 horizontal lines in the middle
  float h1 = wY1 + (wY2 - wY1) * 0.33f;
  float h2 = wY1 + (wY2 - wY1) * 0.66f;
  glVertex3f(wX1, h1, rZ2 - 0.006f);
  glVertex3f(wX2, h1, rZ2 - 0.006f);
  glVertex3f(wX1, h2, rZ2 - 0.006f);
  glVertex3f(wX2, h2, rZ2 - 0.006f);
  // Outer vertical borders (thin)
  glVertex3f(wX1, wY1, rZ2 - 0.006f);
  glVertex3f(wX1, wY2, rZ2 - 0.006f);
  glVertex3f(wX2, wY1, rZ2 - 0.006f);
  glVertex3f(wX2, wY2, rZ2 - 0.006f);
  glEnd();

  // Bold vertical lines (3 middle lines dividing into 4 panes)
  // Using GL_QUADS to make them explicitly thick and bold
  glBegin(GL_QUADS);
  for (int i = 1; i <= 3; i++) {
    float wx = wX1 + i * (wX2 - wX1) / 4.0f;
    float halfThickness = 0.02f; // Thick bold line
    glVertex3f(wx - halfThickness, wY1, rZ2 - 0.007f);
    glVertex3f(wx + halfThickness, wY1, rZ2 - 0.007f);
    glVertex3f(wx + halfThickness, wY2, rZ2 - 0.007f);
    glVertex3f(wx - halfThickness, wY2, rZ2 - 0.007f);
  }
  glEnd();

  // Wall Speaker (Mounted flat on the back wall between the window and corner)
  solidBox(2.4f, 1.0f, rZ2 - 0.1f, 2.7f, 1.3f, rZ2, 0.1f, 0.1f, 0.1f, 0.05f,
           0.05f, 0.05f, 0.15f, 0.15f, 0.15f);

  // Fluorescent Lights on ceiling
  glDisable(GL_LIGHTING);
  glColor3f(1.0f, 1.0f, 0.95f);
  for (float lz = rZ1 + 2.0f; lz <= rZ2 - 1.0f; lz += 2.5f) {
    for (float lx = -1.5f; lx <= 1.5f; lx += 3.0f) {
      solidBox(lx - 0.6f, rY2 - 0.05f, lz - 0.1f, lx + 0.6f, rY2, lz + 0.1f,
               1.0f, 1.0f, 0.95f, 0.9f, 0.9f, 0.9f, 1.0f, 1.0f, 1.0f);
    }
  }
  glEnable(GL_LIGHTING);

  // Door at the front (Shifted to left, next to whiteboard)
  glPushMatrix();
  glTranslatef(-2.5f, 0.01f, rZ1 + 0.01f);
  glRotatef(0, 0, 1, 0); // Face inside
  drawDetailedDoor(0.0f, 0.0f, 0.0f, targetDoorAngle);
  glPopMatrix();

  // (Cable Trunking removed)

  // Whiteboard (Starts near the door, extends past the middle)
  solidBox(-2.0f, 0.2f, rZ1 + 0.01f, 0.6f, 1.0f, rZ1 + 0.05f, 0.95f, 0.95f,
           0.95f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f);

  // Projection Screen (Pulled down, overlapping whiteboard)
  solidBox(-1.5f, 0.5f, rZ1 + 0.06f, 0.2f, 1.3f, rZ1 + 0.07f, 0.9f, 0.9f, 0.9f,
           0.8f, 0.8f, 0.8f, 0.9f, 0.9f, 0.9f);

  // Draw slide header text "Computer Graphics" in dark grey on the projection screen!
  glDisable(GL_LIGHTING);
  glPushMatrix();
  glTranslatef(-1.08f, 0.85f, rZ1 + 0.075f); // Centered 3D coordinate on slide
  glScalef(0.0007f, 0.0007f, 0.0007f);      // Scale down 100-unit stroke size to fit screen
  glLineWidth(2.0f);                        // Bold, crisp lines
  glColor3f(0.1f, 0.15f, 0.2f);             // Dark charcoal text
  for (char c : std::string("Computer Graphics")) {
    glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
  }
  glLineWidth(1.0f); // Reset line width
  glPopMatrix();
  glEnable(GL_LIGHTING);
  // Screen Roller Housing (Black)
  solidBox(-1.6f, 1.3f, rZ1 + 0.05f, 0.3f, 1.4f, rZ1 + 0.09f, 0.1f, 0.1f, 0.1f,
           0.05f, 0.05f, 0.05f, 0.15f, 0.15f, 0.15f);

  // Beam-Mounted Projector
  float pX = -0.5f; // Offset to the left
  float pY = 1.15f; // Hanging below the 1.3f beam
  float pZ =
      rZ1 + 1.2f; // Mounted at the front face of the existing ceiling beam
  // Projector Body
  solidBox(pX - 0.15f, pY, pZ - 0.1f, pX + 0.15f, pY + 0.1f, pZ + 0.2f, 0.8f,
           0.8f, 0.8f, 0.7f, 0.7f, 0.7f, 0.9f, 0.9f, 0.9f);
  // Projector Lens (Facing front wall, so -Z direction)
  solidBox(pX - 0.05f, pY + 0.02f, pZ - 0.12f, pX + 0.05f, pY + 0.08f,
           pZ - 0.1f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f, 0.05f, 0.2f, 0.2f, 0.2f);
  // Mount bracket (Connecting to bottom of beam at Y = 1.3f)
  solidBox(pX - 0.02f, pY + 0.1f, pZ, pX + 0.02f, 1.3f, pZ + 0.1f, 0.7f, 0.7f,
           0.7f, 0.6f, 0.6f, 0.6f, 0.8f, 0.8f, 0.8f);
  // Messy cables hanging and running along the beam
  glColor3f(0.1f, 0.1f, 0.1f);
  glLineWidth(2.0f);
  glBegin(GL_LINE_STRIP);
  glVertex3f(pX, pY + 0.1f, pZ + 0.05f);          // from projector
  glVertex3f(pX + 0.05f, pY + 0.05f, pZ + 0.05f); // dangle down
  glVertex3f(pX + 0.1f, pY + 0.1f, pZ + 0.05f);   // back up
  glVertex3f(pX + 0.1f, 1.25f, pZ + 0.05f);       // up to beam
  glVertex3f(rX1, 1.25f,
             pZ + 0.05f); // run along beam to the LEFT wall (-X side)
  glEnd();

  // TV
  solidBox(0.6f, 0.3f, rZ1 + 0.01f, 1.8f, 0.9f, rZ1 + 0.06f, 0.1f, 0.1f, 0.1f,
           0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f);

  // TV Stand
  solidBox(0.9f, 0.0f, rZ1 + 0.02f, 1.0f, 0.3f, rZ1 + 0.05f, 0.1f, 0.1f, 0.1f,
           0.05f, 0.05f, 0.05f, 0.1f, 0.1f, 0.1f);
  solidBox(1.4f, 0.0f, rZ1 + 0.02f, 1.5f, 0.3f, rZ1 + 0.05f, 0.1f, 0.1f, 0.1f,
           0.05f, 0.05f, 0.05f, 0.1f, 0.1f, 0.1f);
  solidBox(0.8f, 0.0f, rZ1 + 0.01f, 1.6f, 0.05f, rZ1 + 0.15f, 0.2f, 0.2f, 0.2f,
           0.15f, 0.15f, 0.15f, 0.25f, 0.25f, 0.25f);

  // Smart Podium (FOCUS Brand Style)
  glPushMatrix();
  glTranslatef(2.3f, 0.0f, rZ1 + 0.6f);
  // Base Cabinet (Metallic Grey)
  solidBox(-0.13f, 0.0f, -0.15f, 0.13f, 0.45f, 0.15f, 0.35f, 0.35f, 0.35f,
           0.25f, 0.25f, 0.25f, 0.45f, 0.45f, 0.45f);
  // Front Speaker Grill (Faces class -> +Z side)
  solidBox(-0.1f, 0.25f, 0.151f, 0.1f, 0.40f, 0.152f, 0.1f, 0.1f, 0.1f, 0.05f,
           0.05f, 0.05f, 0.15f, 0.15f, 0.15f);
  // Back Glass Door for Server Rack (Faces teacher -> -Z side)
  solidBox(-0.1f, 0.05f, -0.152f, 0.1f, 0.40f, -0.151f, 0.05f, 0.05f, 0.1f,
           0.02f, 0.02f, 0.05f, 0.1f, 0.1f, 0.2f);
  // Slanted Top Desk (Black/Dark Grey)
  glPushMatrix();
  glTranslatef(0.0f, 0.45f, 0.0f);
  glRotatef(-15.0f, 1.0f, 0.0f, 0.0f); // Slant down towards teacher (-Z)
  // Main top board
  solidBox(-0.2f, 0.0f, -0.2f, 0.2f, 0.05f, 0.15f, 0.15f, 0.15f, 0.15f, 0.1f,
           0.1f, 0.1f, 0.2f, 0.2f, 0.2f);
  // Front thick rounded bumper (Audience side -> +Z edge, highest part)
  solidBox(-0.22f, -0.02f, 0.15f, 0.22f, 0.07f, 0.22f, 0.08f, 0.08f, 0.08f,
           0.05f, 0.05f, 0.05f, 0.1f, 0.1f, 0.1f);
  // Small lip at the teacher side (-Z edge) to stop papers sliding off
  solidBox(-0.2f, 0.05f, -0.2f, 0.2f, 0.07f, -0.18f, 0.1f, 0.1f, 0.1f, 0.05f,
           0.05f, 0.05f, 0.15f, 0.15f, 0.15f);
  glPopMatrix(); // End top desk
  glPopMatrix(); // End podium
  glDisable(GL_LIGHTING);
  glColor3f(0.2f, 0.4f, 0.8f);
  glBegin(GL_QUADS);
  glVertex3f(0.62f, 0.32f, rZ1 + 0.061f);
  glVertex3f(1.78f, 0.32f, rZ1 + 0.061f);
  glVertex3f(1.78f, 0.88f, rZ1 + 0.061f);
  glVertex3f(0.62f, 0.88f, rZ1 + 0.061f);
  glEnd();
  glEnable(GL_LIGHTING);

  // (Legacy projector removed to avoid variable redefinition errors and
  // conflicts with the new hanging projector)

  // Teacher (Standing and Teaching)
  if (currentMode != MODE_MORNING_CINEMATIC && !isNight) {
    drawStudent(-0.5f, 0.0f, rZ1 + 0.8f, 0.0f, studentWalkAngle, false, 99, true);
  }

  // Desks, Chairs and Students
  int studentIdx = 0;
  for (float dx = -2.0f; dx <= 2.0f; dx += 1.0f) {
    for (float dz = -5.5f; dz <= 0.0f; dz += 0.9f) {
      // --- Desk ---
      // Top (Light Wood)
      solidBox(dx - 0.35f, 0.3f, dz - 0.2f, dx + 0.35f, 0.35f, dz + 0.1f, 0.8f,
               0.7f, 0.5f, 0.7f, 0.6f, 0.4f, 0.85f, 0.75f, 0.55f);
      // Metal Frame / Legs (Grey)
      solidBox(dx - 0.3f, 0.0f, dz - 0.1f, dx - 0.25f, 0.3f, dz + 0.0f, 0.3f,
               0.3f, 0.3f, 0.25f, 0.25f, 0.25f, 0.35f, 0.35f, 0.35f);
      solidBox(dx + 0.25f, 0.0f, dz - 0.1f, dx + 0.3f, 0.3f, dz + 0.0f, 0.3f,
               0.3f, 0.3f, 0.25f, 0.25f, 0.25f, 0.35f, 0.35f, 0.35f);

      // --- Chair ---
      // Seat (Light Wood)
      solidBox(dx - 0.15f, 0.12f, dz + 0.15f, dx + 0.15f, 0.15f, dz + 0.35f,
               0.8f, 0.7f, 0.5f, 0.7f, 0.6f, 0.4f, 0.85f, 0.75f, 0.55f);
      // Backrest (Light Wood)
      solidBox(dx - 0.15f, 0.35f, dz + 0.35f, dx + 0.15f, 0.45f, dz + 0.4f,
               0.8f, 0.7f, 0.5f, 0.7f, 0.6f, 0.4f, 0.85f, 0.75f, 0.55f);
      // Metal Legs
      solidBox(dx - 0.1f, 0.0f, dz + 0.2f, dx - 0.05f, 0.12f, dz + 0.3f, 0.3f,
               0.3f, 0.3f, 0.25f, 0.25f, 0.25f, 0.35f, 0.35f, 0.35f);
      solidBox(dx + 0.05f, 0.0f, dz + 0.2f, dx + 0.1f, 0.12f, dz + 0.3f, 0.3f,
               0.3f, 0.3f, 0.25f, 0.25f, 0.25f, 0.35f, 0.35f, 0.35f);
      // Metal Back Support
      solidBox(dx - 0.05f, 0.15f, dz + 0.3f, dx + 0.05f, 0.35f, dz + 0.35f,
               0.3f, 0.3f, 0.3f, 0.25f, 0.25f, 0.25f, 0.35f, 0.35f, 0.35f);

      // Seated Student (Keep classroom completely empty during morning cinematic until it finishes)
      // Also hide students at night!
      bool hideAllSeated = (currentMode == MODE_MORNING_CINEMATIC) || isNight;
      if (!hideAllSeated) {
        drawStudent(dx, -0.1f, dz + 0.2f, 180.0f, 0.0f, true, studentIdx++);
      } else {
        studentIdx++;
      }
    }
  }
}

//           sky helpers

void drawStar(float x, float y, float z) {
  glPointSize(2.0f);
  glBegin(GL_POINTS);
  glVertex3f(x, y, z);
  glEnd();
}

void drawStars() {
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

void drawMoon(float angle) {
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

void drawSun(float angle) {
  glDisable(GL_LIGHTING);
  glPushMatrix();
  glRotatef(angle, 0, 0, 1);
  glTranslatef(0.0f, 30.0f, -60.0f);

  // --- Rays (drawn first, behind the sun body) ---
  int numRays = 16;
  float innerR = 3.2f;    // where ray starts (just outside core)
  float outerR = 8.5f;    // where long rays end
  float shortR = 6.0f;    // where short rays end
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

void drawCloud(float x, float y, float z, float scale) {
  glDisable(GL_LIGHTING);
  if (isNight)
    glColor3f(0.25f, 0.28f, 0.35f); // Dark blue-grey at night
  else
    glColor3f(1.0f, 1.0f, 1.0f); // Bright white in day

  glPushMatrix();
  glTranslatef(x, y, z);
  glScalef(scale, scale * 0.6f, scale);

  // Cluster of spheres for fluffy cloud
  float offX[] = {0.0f, 1.2f, -1.2f, 0.7f, -0.7f, 2.0f, -2.0f};
  float offY[] = {0.5f, 0.2f, 0.2f, 0.8f, 0.8f, 0.0f, 0.0f};
  float offZ[] = {0.0f, 0.3f, 0.3f, -0.2f, -0.2f, 0.5f, 0.5f};
  float rad[] = {1.2f, 1.0f, 1.0f, 0.9f, 0.9f, 0.8f, 0.8f};

  for (int i = 0; i < 7; i++) {
    glPushMatrix();
    glTranslatef(offX[i], offY[i], offZ[i]);
    glutSolidSphere(rad[i], 8, 6);
    glPopMatrix();
  }
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

void drawIntroText() {
  if (currentMode != MODE_MORNING_CINEMATIC || introTextAlpha <= 0.001f)
    return;

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST); // Disable depth testing to render overlay directly on top!
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Switch to 2D Orthographic Projection for pixel-perfect text layout
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  // We'll define coordinate system 0 to 800 width, 0 to 600 height
  gluOrtho2D(0, 800, 0, 600);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Draw semi-transparent black background behind the intro text to mask out a bit of the world
  glColor4f(0.0f, 0.0f, 0.0f, introTextAlpha * 0.4f);
  glBegin(GL_QUADS);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(800.0f, 0.0f);
  glVertex2f(800.0f, 600.0f);
  glVertex2f(0.0f, 600.0f);
  glEnd();

  // Render "Good Morning" text
  glColor4f(1.0f, 1.0f, 0.95f, introTextAlpha);

  // Measure text length roughly to center it (each char of 9x15 is 9 pixels wide)
  std::string label = "Good Morning";
  int textWidth = label.length() * 9;
  
  // Position raster cursor centered: X = 400 - (width/2), Y = 300 - (height/2)
  glRasterPos2i(400 - (textWidth / 2), 300 - 7);
  for (char c : label) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
  }

  // Restore 3D matrices
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST); // Re-enable depth testing for subsequent 3D render passes!
  glEnable(GL_LIGHTING);
}

void drawBirds() {
  if (currentMode != MODE_MORNING_CINEMATIC || birdProgress <= 0.001f)
    return;

  glDisable(GL_LIGHTING);
  glPushMatrix();

  // Birds fly across the sky in a V-formation, from high left to low right
  // We will position the center of the flock based on birdProgress (0.0 to 1.0)
  float flockX = -25.0f + (birdProgress * 50.0f);
  float flockY = 16.0f - (birdProgress * 4.0f);
  float flockZ = -35.0f - (birdProgress * 5.0f);

  // Flock offsets for V-formation: 3 birds
  float birdOffsetsX[3] = {0.0f, -1.8f, -1.8f};
  float birdOffsetsY[3] = {0.0f, 0.8f, -0.8f};
  float birdOffsetsZ[3] = {0.0f, 1.2f, -1.2f};

  // Silhouette color against sunrise
  glColor3f(0.08f, 0.08f, 0.1f);

  for (int i = 0; i < 3; i++) {
    glPushMatrix();
    glTranslatef(flockX + birdOffsetsX[i], flockY + birdOffsetsY[i], flockZ + birdOffsetsZ[i]);
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f); // slight forward rotation

    // Body
    glBegin(GL_TRIANGLES);
    // Head/beak
    glVertex3f(0.0f, 0.0f, 0.15f);
    glVertex3f(-0.06f, 0.0f, -0.15f);
    glVertex3f(0.06f, 0.0f, -0.15f);
    glEnd();

    // Wings flapping
    float flapPhase = studentWalkAngle * 2.5f + (i * 1.5f);
    float flapAngle = sinf(flapPhase) * 45.0f; // flap wings between -45 and 45 degrees

    // Left Wing
    glPushMatrix();
    glRotatef(flapAngle, 0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, 0.1f, -0.05f);
    glVertex3f(0.0f, 0.0f, -0.1f);
    glEnd();
    glPopMatrix();

    // Right Wing
    glPushMatrix();
    glRotatef(-flapAngle, 0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.5f, 0.1f, -0.05f);
    glVertex3f(0.0f, 0.0f, -0.1f);
    glEnd();
    glPopMatrix();

    glPopMatrix();
  }

  glPopMatrix();
  glEnable(GL_LIGHTING);
}

void drawSky() {
  glDisable(GL_LIGHTING);

  // Stars (night only)
  if (currentMode == MODE_MORNING_CINEMATIC) {
     float sunPhase = morningProgress / 0.25f;
     if (sunPhase < 0.5f) drawStars(); // fade out halfway through sunrise
  } else if (isNight) {
     drawStars();
  }

  // Sun or Moon
  if (currentMode == MODE_MORNING_CINEMATIC) {
     float sunPhase = morningProgress / 0.25f; // 0.0 to 1.0
     if (sunPhase > 1.0f) sunPhase = 1.0f;
     float cinematicSunAngle = -120.0f + (150.0f * sunPhase); // Rises from deep below horizon
     drawSun(cinematicSunAngle);
  } else if (isNight) {
     drawMoon(sunMoonAngle);
  } else {
     drawSun(sunMoonAngle);
  }

  // Draw Flying Birds
  drawBirds();

  // Clouds — 5 clouds drifting across the sky
  float baseZ = -50.0f;
  drawCloud(-30.0f + cloudOffset, 18.0f, baseZ, 1.8f);
  drawCloud(-5.0f + cloudOffset, 22.0f, baseZ - 10.f, 1.4f);
  drawCloud(20.0f + cloudOffset, 19.0f, baseZ + 5.f, 2.0f);
  drawCloud(-50.0f + cloudOffset, 20.0f, baseZ - 5.f, 1.6f);
  drawCloud(45.0f + cloudOffset, 21.0f, baseZ, 1.5f);

  glEnable(GL_LIGHTING);
}

void drawCar(float x, float y, float z, float angle, int type, float r, float g,
             float b) {
  glPushMatrix();
  glTranslatef(x, y, z);
  glRotatef(angle, 0, 1, 0);

  if (type == 0) { // Sedan
    // Lower Body
    glColor3f(r, g, b);
    solidBox(-0.6f, 0.1f, -1.2f, 0.6f, 0.45f, 1.2f, r, g, b, r * 0.8f, g * 0.8f,
             b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
    // Upper Cabin / Roof
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    solidBox(-0.55f, 0.45f, -0.5f, 0.55f, 0.85f, 0.6f, r * 0.7f, g * 0.7f,
             b * 0.7f, r * 0.6f, g * 0.6f, b * 0.6f, r * 0.9f, g * 0.9f,
             b * 0.9f);
  } else if (type == 1) { // SUV
    // Lower Body (bulkier and taller)
    glColor3f(r, g, b);
    solidBox(-0.65f, 0.12f, -1.3f, 0.65f, 0.58f, 1.3f, r, g, b, r * 0.8f,
             g * 0.8f, b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
    // Boxy Cabin
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    solidBox(-0.60f, 0.58f, -0.8f, 0.60f, 1.0f, 0.7f, r * 0.7f, g * 0.7f,
             b * 0.7f, r * 0.6f, g * 0.6f, b * 0.6f, r * 0.9f, g * 0.9f,
             b * 0.9f);
  } else if (type == 2) { // Pickup Truck
    // Lower Cabin/Chassis
    glColor3f(r, g, b);
    solidBox(-0.65f, 0.12f, -1.4f, 0.65f, 0.52f, 1.4f, r, g, b, r * 0.8f,
             g * 0.8f, b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
    // Cab (Front portion)
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    solidBox(-0.60f, 0.52f, 0.1f, 0.60f, 0.95f, 1.0f, r * 0.7f, g * 0.7f,
             b * 0.7f, r * 0.6f, g * 0.6f, b * 0.6f, r * 0.9f, g * 0.9f,
             b * 0.9f);
    // Truck Bed Walls (Grey/Metallic interior)
    glColor3f(0.25f, 0.25f, 0.25f);
    solidBox(-0.65f, 0.52f, -1.4f, -0.58f, 0.72f, 0.1f, 0.25f, 0.25f, 0.25f,
             0.2f, 0.2f, 0.2f, 0.3f, 0.3f, 0.3f);
    solidBox(0.58f, 0.52f, -1.4f, 0.65f, 0.72f, 0.1f, 0.25f, 0.25f, 0.25f, 0.2f,
             0.2f, 0.2f, 0.3f, 0.3f, 0.3f);
    solidBox(-0.65f, 0.52f, -1.4f, 0.65f, 0.72f, -1.33f, 0.25f, 0.25f, 0.25f,
             0.2f, 0.2f, 0.2f, 0.3f, 0.3f, 0.3f);
  } else if (type == 3) { // Sports Car
    // Low sleek lower body
    glColor3f(r, g, b);
    solidBox(-0.68f, 0.08f, -1.35f, 0.68f, 0.36f, 1.35f, r, g, b, r * 0.8f,
             g * 0.8f, b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
    // Slanted narrow cabin
    glColor3f(0.1f, 0.1f, 0.1f);
    solidBox(-0.55f, 0.36f, -0.4f, 0.55f, 0.65f, 0.4f, 0.1f, 0.1f, 0.1f, 0.08f,
             0.08f, 0.08f, 0.12f, 0.12f, 0.12f);
    // Rear spoiler wing
    glColor3f(r, g, b);
    solidBox(-0.62f, 0.46f, -1.3f, 0.62f, 0.50f, -1.15f, r, g, b, r * 0.8f,
             g * 0.8f, b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
    // Wing mounts
    glColor3f(0.1f, 0.1f, 0.1f);
    solidBox(-0.55f, 0.36f, -1.25f, -0.50f, 0.46f, -1.20f, 0.1f, 0.1f, 0.1f,
             0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
    solidBox(0.50f, 0.36f, -1.25f, 0.55f, 0.46f, -1.20f, 0.1f, 0.1f, 0.1f, 0.1f,
             0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
  }

  else if (type == 4) { // Bajaj (3-wheeled auto-rickshaw)
    // Lower Body (Chassis) - narrow and cute
    float br = 0.1f, bg = 0.4f, bb = 0.9f; // Iconic royal blue
    // Front slanted nose/fairing
    solidBox(-0.35f, 0.12f, 0.3f, 0.35f, 0.55f, 0.85f, br, bg, bb, br * 0.8f,
             bg * 0.8f, bb * 0.8f, br * 1.1f, bg * 1.1f, bb * 1.1f);
    // Rear passenger tub
    solidBox(-0.45f, 0.12f, -0.9f, 0.45f, 0.50f, 0.3f, br, bg, bb, br * 0.8f,
             bg * 0.8f, bb * 0.8f, br * 1.1f, bg * 1.1f, bb * 1.1f);
    // Yellow Canvas Roof Canopy supported by thin black pillars
    float ry = 0.9f, rg = 0.8f, rb = 0.1f; // Mustard Yellow
    solidBox(-0.45f, 0.88f, -0.9f, 0.45f, 0.95f, 0.4f, ry, rg, rb, ry * 0.8f,
             rg * 0.8f, rb * 0.8f, ry * 1.1f, rg * 1.1f, rb * 1.1f);
    // Thin support pillars (Black)
    solidBox(-0.43f, 0.50f, -0.88f, -0.40f, 0.88f, -0.85f, 0.1f, 0.1f, 0.1f,
             0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
    solidBox(0.40f, 0.50f, -0.88f, 0.43f, 0.88f, -0.85f, 0.1f, 0.1f, 0.1f, 0.1f,
             0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
    solidBox(-0.43f, 0.50f, 0.25f, -0.40f, 0.88f, 0.28f, 0.1f, 0.1f, 0.1f, 0.1f,
             0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
    solidBox(0.40f, 0.50f, 0.25f, 0.43f, 0.88f, 0.28f, 0.1f, 0.1f, 0.1f, 0.1f,
             0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
    // Front center windshield support
    solidBox(-0.02f, 0.55f, 0.78f, 0.02f, 0.88f, 0.82f, 0.1f, 0.1f, 0.1f, 0.1f,
             0.1f, 0.1f, 0.1f, 0.1f, 0.1f);
  }

  // 3. Windows (Dark Blue-Grey tint)
  glColor3f(0.15f, 0.2f, 0.3f);
  glBegin(GL_QUADS);
  if (type == 4) { // Bajaj windshield
    glVertex3f(-0.32f, 0.55f, 0.80f);
    glVertex3f(0.32f, 0.55f, 0.80f);
    glVertex3f(0.32f, 0.88f, 0.42f);
    glVertex3f(-0.32f, 0.88f, 0.42f);
  } else if (type != 3) { // Sports car has fully tinted glass canopy built-in
    // Front Windshield
    glVertex3f(-0.5f, 0.45f, 0.61f);
    glVertex3f(0.5f, 0.45f, 0.61f);
    glVertex3f(0.45f, 0.78f, 0.52f);
    glVertex3f(-0.45f, 0.78f, 0.52f);
    // Back Window
    glVertex3f(-0.5f, 0.45f, -0.51f);
    glVertex3f(0.5f, 0.45f, -0.51f);
    glVertex3f(0.45f, 0.78f, -0.42f);
    glVertex3f(-0.45f, 0.78f, -0.42f);
  } else {
    // Sleek front/back windshields for sports car
    glVertex3f(-0.48f, 0.36f, 0.41f);
    glVertex3f(0.48f, 0.36f, 0.41f);
    glVertex3f(0.42f, 0.60f, 0.32f);
    glVertex3f(-0.42f, 0.60f, 0.32f);
    glVertex3f(-0.48f, 0.36f, -0.41f);
    glVertex3f(0.48f, 0.36f, -0.41f);
    glVertex3f(0.42f, 0.60f, -0.32f);
    glVertex3f(-0.42f, 0.60f, -0.32f);
  }
  glEnd();

  // 4. Wheels (Dark Grey with silver rims)
  glColor3f(0.1f, 0.1f, 0.1f);
  float wheelX[4];
  float wheelZ[4];
  int numWheels = 4;
  if (type == 4) {
    numWheels = 3;
    wheelX[0] = 0.0f;
    wheelZ[0] = 0.70f; // Front Center
    wheelX[1] = -0.42f;
    wheelZ[1] = -0.65f; // Rear Left
    wheelX[2] = 0.42f;
    wheelZ[2] = -0.65f; // Rear Right
  } else {
    wheelX[0] = -0.6f;
    wheelZ[0] = 0.75f;
    wheelX[1] = 0.6f;
    wheelZ[1] = 0.75f;
    wheelX[2] = -0.6f;
    wheelZ[2] = -0.75f;
    wheelX[3] = 0.6f;
    wheelZ[3] = -0.75f;
  }

  for (int i = 0; i < numWheels; i++) {
    glPushMatrix();
    glTranslatef(wheelX[i], type == 4 ? 0.15f : 0.18f, wheelZ[i]);
    glRotatef(90, 0, 1, 0);
    if (type != 4 && i % 2 == 0)
      glRotatef(180, 0, 1, 0);
    // Tire
    glColor3f(0.15f, 0.15f, 0.15f);
    gluCylinder(qobj, type == 4 ? 0.15 : 0.18, type == 4 ? 0.15 : 0.18,
                type == 4 ? 0.10 : 0.15, 16, 2);
    // Rim
    glTranslatef(0, 0, type == 4 ? 0.101f : 0.151f);
    glColor3f(0.6f, 0.6f, 0.65f);
    gluDisk(qobj, 0, type == 4 ? 0.09 : 0.12, 16, 2);
    glPopMatrix();
  }

  // 5. Headlights & Tail lights
  if (type == 4) {
    // Bajaj Single Front Headlight
    glColor3f(1.0f, 1.0f, 0.8f);
    solidBox(-0.08f, 0.30f, 0.85f, 0.08f, 0.40f, 0.87f, 1.0f, 1.0f, 0.8f, 0.9f,
             0.9f, 0.7f, 1.0f, 1.0f, 0.9f);
    // Bajaj Tail lights
    glColor3f(0.8f, 0.0f, 0.0f);
    solidBox(-0.35f, 0.22f, -0.92f, -0.25f, 0.30f, -0.90f, 0.8f, 0.0f, 0.0f,
             0.7f, 0.0f, 0.0f, 0.9f, 0.1f, 0.1f);
    solidBox(0.25f, 0.22f, -0.92f, 0.35f, 0.30f, -0.90f, 0.8f, 0.0f, 0.0f, 0.7f,
             0.0f, 0.0f, 0.9f, 0.1f, 0.1f);
  } else {
    // Front (White/Yellow)
    glColor3f(1.0f, 1.0f, 0.8f);
    solidBox(-0.55f, 0.22f, 1.2f, -0.35f, 0.32f, 1.22f, 1.0f, 1.0f, 0.8f, 0.9f,
             0.9f, 0.7f, 1.0f, 1.0f, 0.9f);
    solidBox(0.35f, 0.22f, 1.2f, 0.55f, 0.32f, 1.22f, 1.0f, 1.0f, 0.8f, 0.9f,
             0.9f, 0.7f, 1.0f, 1.0f, 0.9f);
    // Back (Red)
    glColor3f(0.8f, 0.0f, 0.0f);
    solidBox(-0.55f, 0.22f, -1.22f, -0.35f, 0.32f, -1.2f, 0.8f, 0.0f, 0.0f,
             0.7f, 0.0f, 0.0f, 0.9f, 0.1f, 0.1f);
    solidBox(0.35f, 0.22f, -1.22f, 0.55f, 0.32f, -1.2f, 0.8f, 0.0f, 0.0f, 0.7f,
             0.0f, 0.0f, 0.9f, 0.1f, 0.1f);
  }

  glPopMatrix();
}

/**
 * @brief Renders the base terrain layer (Soil and Grass)
 */
void renderBaseTerrain() {
  auto drawGroundPatch = [&](float x1, float z1, float x2, float z2) {
    // Soil
    glColor3f(0.18f, 0.15f, 0.12f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(x1, -0.03f, z2);
    glVertex3f(x2, -0.03f, z2);
    glVertex3f(x2, -0.03f, z1);
    glVertex3f(x1, -0.03f, z1);
    glEnd();

    // Grass
    glColor3f(CampusConfig::COLOR_GRASS[0], CampusConfig::COLOR_GRASS[1],
              CampusConfig::COLOR_GRASS[2]);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(x1, -0.02f, z2);
    glVertex3f(x2, -0.02f, z2);
    glVertex3f(x2, -0.02f, z1);
    glVertex3f(x1, -0.02f, z1);
    glEnd();
  };

  float gs = CampusConfig::GROUND_SIZE; // 100.0f

  // We slice the terrain along Z to cut actual holes for the deep ditches
  // so they aren't covered by the flat grass quad.

  // 1. Far back area: Z from -gs to 1.85f (Behind front ditch)
  drawGroundPatch(-gs, -gs, gs, 1.85f);

  // 2. Front Ditch area: Z from 1.85f to 2.35f
  // Front ditch runs from X = -6.0 to 6.0. We draw grass on the left and right.
  drawGroundPatch(-gs, 1.85f, -6.0f, 2.35f);
  drawGroundPatch(6.0f, 1.85f, gs, 2.35f);

  // 3. Middle area between the two ditches: Z from 2.35f to 9.0f
  drawGroundPatch(-gs, 2.35f, gs, 9.0f);

  // 4. Main Ditch area: Z from 9.0f to 10.0f
  // The main ditch spans across but has gaps for 3 paths. We draw grass in
  // those gaps.
  drawGroundPatch(-25.75f, 9.0f, -22.75f, 10.0f); // Left building path
  drawGroundPatch(-9.0f, 9.0f, -6.0f, 10.0f);     // Center building left path
  drawGroundPatch(7.75f, 9.0f, 10.75f, 10.0f);    // Center building right path

  // 5. Far front area: Z from 10.0f to gs (Up to the road)
  drawGroundPatch(-gs, 10.0f, gs, gs);
}

/**
 * @brief Renders the asphalt road with markings and weathering effects
 */
void renderCampusRoad() {
  // Main Asphalt Surface
  glColor3f(CampusConfig::COLOR_ASPHALT[0], CampusConfig::COLOR_ASPHALT[1],
            CampusConfig::COLOR_ASPHALT[2]);
  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);
  glVertex3f(-CampusConfig::GROUND_SIZE, CampusConfig::ROAD_Y,
             CampusConfig::ROAD_Z_START);
  glVertex3f(CampusConfig::GROUND_SIZE, CampusConfig::ROAD_Y,
             CampusConfig::ROAD_Z_START);
  glVertex3f(CampusConfig::GROUND_SIZE, CampusConfig::ROAD_Y,
             CampusConfig::ROAD_Z_END);
  glVertex3f(-CampusConfig::GROUND_SIZE, CampusConfig::ROAD_Y,
             CampusConfig::ROAD_Z_END);
  glEnd();

  // Road Weathering (Patches & Manholes)
  for (float x = -80.0f; x <= 80.0f; x += 35.0f) {
    // Deterministic offset based on x position to avoid shaking every frame
    int seed = (int)((x + 100.0f) * 10.0f);
    float offZ = (float)(seed % 20) / 10.0f - 1.0f;
    glColor3f(0.18f, 0.18f, 0.20f);
    glBegin(GL_QUADS);
    glVertex3f(x - 0.8f, -0.014f, 14.5f + offZ - 0.4f);
    glVertex3f(x + 1.2f, -0.014f, 14.5f + offZ - 0.4f);
    glVertex3f(x + 1.2f, -0.014f, 14.5f + offZ + 0.5f);
    glVertex3f(x - 0.8f, -0.014f, 14.5f + offZ + 0.5f);
    glEnd();
    glColor3f(0.3f, 0.3f, 0.32f);
    glPushMatrix();
    glTranslatef(x + 5.0f, -0.013f, 15.5f);
    glScalef(1.0f, 0.1f, 1.0f);
    glutSolidSphere(0.35f, 8, 4);
    glPopMatrix();
  }

  // Zebra Striped Curbs with ROUNDED ENTRANCES
  float segmentLen = 1.2f;
  float radius = 0.6f;

  auto drawCurbWithGaps = [&](float zPos, bool isFront) {
    // 1. Draw Straight segments
    for (float x = -CampusConfig::GROUND_SIZE; x < CampusConfig::GROUND_SIZE;
         x += segmentLen) {
      bool inPath = false;
      if (isFront) {
        // Check if this segment overlaps with any path entrance (including the
        // radius)
        if ((x + segmentLen > -9.0f - radius && x < -6.0f + radius) ||
            (x + segmentLen > 7.75f - radius && x < 10.75f + radius) ||
            (x + segmentLen > -25.75f - radius && x < -22.75f + radius)) {
          inPath = true;
        }
      }

      if (!inPath) {
        bool isWhite = ((int)((x + 100.1f) / segmentLen) % 2 == 0);
        float r = isWhite ? 0.95f : 0.15f;
        float g = isWhite ? 0.95f : 0.15f;
        float b = isWhite ? 0.95f : 0.15f;
        solidBox(x, 0.0f, zPos, x + segmentLen, 0.18f, zPos + 0.35f, r, g, b,
                 r * 0.8f, g * 0.8f, b * 0.8f, r * 1.1f, g * 1.1f, b * 1.1f);
      }
    }

    // 2. Draw Rounded Corners (Front only)
    if (isFront) {
      // Path 1
      drawRoundedCorner(-9.0f - radius, zPos - radius, radius, 0, 90, true);
      drawRoundedCorner(-6.0f + radius, zPos - radius, radius, 90, 90, false);
      // Path 2
      drawRoundedCorner(7.75f - radius, zPos - radius, radius, 0, 90, true);
      drawRoundedCorner(10.75f + radius, zPos - radius, radius, 90, 90, false);
      // Path 3
      drawRoundedCorner(-25.75f - radius, zPos - radius, radius, 0, 90, true);
      drawRoundedCorner(-22.75f + radius, zPos - radius, radius, 90, 90, false);
    }
  };

  drawCurbWithGaps(11.65f, true); // Front Curb
  drawCurbWithGaps(17.0f, false); // Back Curb
}

/**
 * @brief Renders the campus walkways, including patterned sidewalks and
 * fan-pattern paths
 */
void renderWalkways() {
  // Patterned Sidewalks ALONG THE ROAD
  drawPatternedSidewalk(-100.0f, 10.5f, 100.0f, 11.65f);
  drawPatternedSidewalk(-100.0f, 17.35f, 100.0f, 18.5f);

  // Side Walkway Centered between buildings - EXTENDED TO ROAD
  drawFanCobblestonePath(-9.0f, -15.0f, -6.0f, 11.65f);
  drawFanCobblestonePath(7.75f, -15.0f, 10.75f, 11.65f);
  drawFanCobblestonePath(-25.75f, -15.0f, -22.75f, 11.65f);

  // Path Entrance Grates (Concrete slabs with gaps near the road)
  drawPathEntranceGrate(-9.0f, -6.0f, 11.65f, 12.0f);
  drawPathEntranceGrate(7.75f, 10.75f, 11.65f, 12.0f);
  drawPathEntranceGrate(-25.75f, -22.75f, 11.65f, 12.0f);

  // Deep Ditch Grates (Concrete slabs bridging the main deep ditch)
  drawPathEntranceGrate(-9.0f, -6.0f, 9.0f, 10.0f);
  drawPathEntranceGrate(7.75f, 10.75f, 9.0f, 10.0f);
  drawPathEntranceGrate(-25.75f, -22.75f, 9.0f, 10.0f);

  // Front Drainage Ditch (horizontal, in front of center building)
  drawDitchHorizontal(2.1f, -6.0f, 6.0f);

  // Horizontal drainage ditch running across the campus between buildings and
  // road
  float ditchZ = 9.5f; // Pushed closer to the road bushes (at 10.2)
  float ditchW = 1.0f; // Increased width
  float ditchD =
      0.85f; // Deep hole (decreased Y-axis / 'Z-axis' in some software)

  // Helper to draw ditch and its concrete border
  auto drawDitchWithBorder = [&](float xStart, float xEnd) {
    drawDitchHorizontal(ditchZ, xStart, xEnd, ditchW, ditchD);
    // Concrete curb on the building side of the ditch (like in the photo)
    solidBox(xStart, 0.0f, ditchZ - ditchW / 2 - 0.2f, xEnd, 0.04f,
             ditchZ - ditchW / 2, 0.65f, 0.65f, 0.62f, 0.55f, 0.55f, 0.52f,
             0.7f, 0.7f, 0.68f);
  };

  // Segment 1: Far left up to left building's path
  drawDitchWithBorder(-100.0f, -25.75f);

  // Segment 2: Between left building's path and center building's left path
  drawDitchWithBorder(-22.75f, -9.0f);

  // Segment 3: Between center building's two paths (in front of center
  // building)
  drawDitchWithBorder(-6.0f, 7.75f);

  // Segment 4: From center building's right path to the far right
  drawDitchWithBorder(10.75f, 100.0f);

  // Solid gray ground strip in front of plinth
  solidBox(-5.8f, -0.01f, 1.8f, 5.8f, 0.01f, 2.5f, 0.45f, 0.45f, 0.45f, 0.4f,
           0.4f, 0.4f, 0.5f, 0.5f, 0.5f);
}

/**
 * @brief Renders the procedural vegetation and lighting fixtures
 */
void renderCampusVegetation() {
  // Road Hedges - DENSIFIED into a "Wall of Green"
  for (float x = -100.0f; x <= 100.0f; x += 0.35f) {
    // Skip hedge where paths intersect
    if ((x > -9.2f && x < -5.8f) || (x > 7.55f && x < 10.95f) ||
        (x > -25.95f && x < -22.55f))
      continue;

    drawBush(x, 10.2f);
    drawBush(x, 18.8f);
  }

  // Building surrounding hedges (Center building)
  for (float x = -6.0f; x <= 4.2f; x += 0.60f)
    drawBush(x, 3.5f);
  for (float z = 4.1f; z <= 8.5f; z += 0.6f)
    drawBush(4.2f, z);
  for (float x = -5.4f; x <= 4.2f; x += 0.6f)
    drawBush(x, 8.4f);
  for (float z = 4.1f; z <= 8.5f; z += 0.6f)
    drawBush(-6.0f, z);

  // Building surrounding hedges (Left building, offset by -15.0)
  for (float x = -6.0f; x <= 4.2f; x += 0.60f)
    drawBush(x - 15.0f, 3.5f);
  for (float z = 4.1f; z <= 8.5f; z += 0.6f)
    drawBush(4.2f - 15.0f, z);
  for (float x = -5.4f; x <= 4.2f; x += 0.6f)
    drawBush(x - 15.0f, 8.4f);
  for (float z = 4.1f; z <= 8.5f; z += 0.6f)
    drawBush(-6.0f - 15.0f, z);

  // Side path trees are now dynamically populated by the global tree grid below

  // Long Grass inside hedged area (Center building)
  for (float x = -5.5f; x <= 3.7f; x += 0.6f) {
    for (float z = 4.0f; z <= 8.2f; z += 0.6f) {
      int ix = (int)((x + 100.0f) * 10.0f);
      int iz = (int)((z + 100.0f) * 10.0f);
      int seed = ix * 1973 + iz * 9277 + 42;
      float jx = (float)((seed & 0xFF) % 10) / 20.0f - 0.25f;
      float jz = (float)(((seed >> 8) & 0xFF) % 10) / 20.0f - 0.25f;
      drawLongGrass(x + jx, z + jz);
    }
  }

  // Long Grass inside hedged area (Left building, offset by -15.0)
  for (float x = -5.5f; x <= 3.7f; x += 0.5f) {
    for (float z = 4.0f; z <= 8.2f; z += 0.5f) {
      int ix = (int)((x - 15.0f + 100.0f) * 10.0f);
      int iz = (int)((z + 100.0f) * 10.0f);
      int seed = ix * 1973 + iz * 99277 + 42;
      float jx = (float)((seed & 0xFF) % 10) / 20.0f - 0.25f;
      float jz = (float)(((seed >> 8) & 0xFF) % 10) / 20.0f - 0.25f;
      drawLongGrass(x - 15.0f + jx, z + jz);
    }
  }

  // Streetlights
  for (float x = -100.0f; x <= 100.0f; x += 15.0f) {
    drawStreetLight(x, 10.0f);
    drawStreetLight(x, 19.0f);
  }

  // Background forest line
    // =========================================================================
    // --- LUSH GREEN CAMPUS ARBORETUM & FORESTS (EXPLICIT PLACEMENTS) ---------
    // =========================================================================

    // --- Zone 1: Behind Road Left Forest Side ---
    drawTree(-45.0f, 19.5f, 0); // Normal Pine (Moved behind back sidewalk)
    drawTree(-42.0f, 20.2f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-39.0f, 19.8f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-36.0f, 20.5f, 0); // Normal Pine (Moved behind back sidewalk)
    drawTree(-33.0f, 19.6f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-30.0f, 20.1f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-27.0f, 19.7f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-24.0f, 20.4f, 0); // Normal Pine (Moved behind back sidewalk)
    drawTree(-21.0f, 19.9f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-18.0f, 20.3f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-15.0f, 19.8f, 1); // Normal Leafy (Moved behind back sidewalk)
    drawTree(-12.0f, 20.0f, 0); // Normal Pine (Moved behind back sidewalk)

    // --- Zone 2: Behind Road Right Forest Side ---
    drawTree(12.0f, 20.0f, 0);  // Normal Pine (Moved behind back sidewalk)
    drawTree(15.0f, 19.8f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(18.0f, 20.3f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(21.0f, 19.9f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(24.0f, 20.4f, 0);  // Normal Pine (Moved behind back sidewalk)
    drawTree(27.0f, 19.7f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(30.0f, 20.1f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(33.0f, 19.6f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(36.0f, 20.5f, 0);  // Normal Pine (Moved behind back sidewalk)
    drawTree(39.0f, 19.8f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(42.0f, 20.2f, 1);  // Normal Leafy (Moved behind back sidewalk)
    drawTree(45.0f, 19.5f, 0);  // Normal Pine (Moved behind back sidewalk)

    // --- Zone 3: Left Walkway Side Gardens (Z adjusted to clear road/ditch, X adjusted to clear paths) ---
    drawTree(-16.0f, 8.0f, 1); // Normal Leafy
    drawTree(-13.0f, 8.2f, 1); // Normal Leafy
    drawTree(-10.5f, 8.0f, 1); // Normal Leafy (Moved to clear vertical path at -9.0 to -6.0)

    // --- Zone 4: Right Walkway Side Gardens (Z adjusted to clear road/ditch, X adjusted to clear paths) ---
    drawTree(  5.0f, 8.1f, 1); // Normal Leafy (Moved to clear vertical path at 7.75 to 10.75)
    drawTree( 12.0f, 8.0f, 1); // Normal Leafy (Moved to clear vertical path at 7.75 to 10.75)
    drawTree( 14.5f, 8.2f, 1); // Normal Leafy
    drawTree( 17.5f, 8.0f, 1); // Normal Leafy

    // --- Zone 5: Academic Left Quad Gardens (Moved to far left lawn to clear Left Building) ---
    drawTree(-22.0f, 2.5f, 1);  // Far Left Lawn
    drawTree(-23.0f, 1.2f, 1);  // Far Left Lawn
    drawTree(-22.5f, -1.5f, 1); // Far Left Lawn
    drawTree(-23.5f, -3.0f, 1); // Far Left Lawn
    drawTree(-22.0f, -5.5f, 1); // Far Left Lawn
    drawTree(-23.0f, -7.2f, 1); // Far Left Lawn

    // --- Zone 6: Academic Right Quad Gardens (Moved to clear Center Building) ---
    drawTree( 6.5f, 2.5f, 1);   // Quad between Main Building and Right Path
    drawTree( 6.2f, 1.2f, 1);   // Quad between Main Building and Right Path
    drawTree(12.0f, -1.5f, 1);  // Far Right Lawn (Clear of Right Path at 10.75)
    drawTree(13.5f, -3.0f, 1);  // Far Right Lawn
    drawTree(12.5f, -5.5f, 1);  // Far Right Lawn
    drawTree(13.0f, -7.2f, 1);  // Far Right Lawn

    // --- Zone 7: Dense Background Forest Wall ---
    drawTree(-50.0f, -12.0f, 0); // Normal Pine
    drawTree(-45.0f, -12.5f, 1); // Normal Leafy
    drawTree(-40.0f, -11.8f, 0); // Normal Pine
    drawTree(-35.0f, -12.2f, 1); // Normal Leafy
    drawTree(-30.0f, -12.0f, 0); // Normal Pine
    drawTree(-25.0f, -12.5f, 1); // Normal Leafy
    drawTree(-20.0f, -11.8f, 0); // Normal Pine
    drawTree(-15.0f, -12.2f, 1); // Normal Leafy
    drawTree(-10.0f, -12.0f, 0); // Normal Pine
    drawTree( -5.0f, -12.5f, 1); // Normal Leafy
    drawTree(  0.0f, -12.0f, 0); // Normal Pine
    drawTree(  5.0f, -12.5f, 1); // Normal Leafy
    drawTree( 10.0f, -12.0f, 0); // Normal Pine
    drawTree( 15.0f, -12.2f, 1); // Normal Leafy
    drawTree( 20.0f, -11.8f, 0); // Normal Pine
    drawTree( 25.0f, -12.5f, 1); // Normal Leafy
    drawTree( 30.0f, -12.0f, 0); // Normal Pine
    drawTree( 35.0f, -12.2f, 1); // Normal Leafy
    drawTree( 40.0f, -11.8f, 0); // Normal Pine
    drawTree( 45.0f, -12.5f, 1); // Normal Leafy
    drawTree( 50.0f, -12.0f, 0); // Normal Pine
}

/**
 * @brief Top-level function to render the entire environment
 */
void drawEarth() {
  renderBaseTerrain();
  renderCampusRoad();
  renderWalkways();
  renderCampusVegetation();
}

//           stairs (U-shaped / Dog-leg)
void drawStaircase(float xOuter, float xInner, float zFront, float zBack) {
  float midX = (xOuter + xInner) * 0.5f;
  float yMid = 0.51f;
  float yTop = 1.00f;
  int N = 6;
  float gap = 0.12f;
  float midX_Inner = midX + (xInner > xOuter ? gap : -gap);
  float midX_Outer = midX + (xOuter > xInner ? gap : -gap);
  float lDepth = 0.55f;
  float density = 0.06f;
  float rSize = 0.015f;
  float rH = 0.45f;                  // railing post height
  bool leftSide = (xOuter < xInner); // true = left staircase

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
    solidBox(midX_Inner, yt - 0.02f, zb, xInner, yt, zf, 0.60f, 0.60f, 0.60f,
             0.50f, 0.50f, 0.50f, 0.65f, 0.65f, 0.65f);
    solidBox(midX_Inner, yb, zb - 0.02f, xInner, yt - 0.02f, zb, 0.50f, 0.50f,
             0.50f, 0.40f, 0.40f, 0.40f, 0.45f, 0.45f, 0.45f);
  }

  // Mid landing platform
  solidBox(xOuter, yMid - 0.02f, zBack - lDepth, xInner, yMid, zBack, 0.60f,
           0.60f, 0.60f, 0.50f, 0.50f, 0.50f, 0.65f, 0.65f, 0.65f);

  // Flight 2: mid landing → top balcony
  for (int i = 0; i < N; i++) {
    float f = (float)i / N;
    float yb = yMid + f * (yTop - yMid);
    float yt = yMid + (float)(i + 1) / N * (yTop - yMid);
    float zb = zBack + f * (zFront - zBack);
    float zf = zBack + (float)(i + 1) / N * (zFront - zBack);
    solidBox(xOuter, yt - 0.02f, zb, midX_Outer, yt, zf, 0.60f, 0.60f, 0.60f,
             0.50f, 0.50f, 0.50f, 0.65f, 0.65f, 0.65f);
    solidBox(xOuter, yb, zb - 0.02f, midX_Outer, yt - 0.02f, zb, 0.50f, 0.50f,
             0.50f, 0.40f, 0.40f, 0.40f, 0.45f, 0.45f, 0.45f);
  }

  // Top landing platform
  solidBox(xOuter, yTop - 0.08f, 0.7f, xInner, yTop, 1.65f, 0.72f, 0.72f, 0.72f,
           0.58f, 0.58f, 0.58f, 0.66f, 0.66f, 0.66f);

  //  ── RAILING POSTS ───────────────────────────────────────────────
  // Helper lambda: draw a single baluster post
  auto post = [&](float x, float y, float z) {
    solidBox(x, y, z, x + outSign * rSize, y + rH, z + rSize, 0.92f, 0.92f,
             0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
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
    solidBox(xOuter + outSign * rSize, yMid, z, xOuter, yMid + rH, z + rSize,
             0.92f, 0.92f, 0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
  // NOTE: No back edge railing (against building wall) and no inner railing
  // (stair opening)

  // Mid-landing: horizontal top rail caps — front and outer side only
  // Front cap
  solidBox(xOuter, yMid + rH - 0.02f, zBack - lDepth - 0.01f, xInner,
           yMid + rH + 0.02f, zBack - lDepth + 0.03f, 0.80f, 0.80f, 0.80f,
           0.70f, 0.70f, 0.70f, 0.85f, 0.85f, 0.85f);
  // Outer side cap
  solidBox(xOuter + outSign * 0.03f, yMid + rH - 0.02f, zBack - lDepth, xOuter,
           yMid + rH + 0.02f, zBack, 0.80f, 0.80f, 0.80f, 0.70f, 0.70f, 0.70f,
           0.85f, 0.85f, 0.85f);

  // Top-landing platform: X from min(xOuter,xInner) to max(xOuter,xInner), Z
  // from 0.7 to 1.65
  float tX1 = (xOuter < xInner) ? xOuter : xInner; // smaller X
  float tX2 = (xOuter < xInner) ? xInner : xOuter; // larger X
  float tZ1 = 0.70f;
  float tZ2 = 1.65f;
  float pH = yTop + rH; // top of post
  float tXout = xOuter; // the real outer X edge (left=-5.7, right=5.7)

  // Front face (z = tZ2, posts along X)
  for (float x = tX1; x <= tX2; x += density)
    solidBox(x, yTop, tZ2, x + rSize, pH, tZ2 + rSize, 0.92f, 0.92f, 0.92f,
             0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
  solidBox(tX1, pH - 0.02f, tZ2, tX2, pH + 0.02f, tZ2 + 0.04f, 0.82f, 0.82f,
           0.82f, 0.72f, 0.72f, 0.72f, 0.88f, 0.88f, 0.88f);

  // Outer side (x = xOuter, posts along Z) — outward direction
  for (float z = tZ1; z <= tZ2; z += density)
    solidBox(tXout, yTop, z, tXout + outSign * rSize, pH, z + rSize, 0.92f,
             0.92f, 0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
  solidBox(tXout, pH - 0.02f, tZ1, tXout + outSign * 0.04f, pH + 0.02f, tZ2,
           0.82f, 0.82f, 0.82f, 0.72f, 0.72f, 0.72f, 0.88f, 0.88f, 0.88f);
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
  glVertex3f(xOuter - 0.05f, 0.0f, 1.75f);
  glVertex3f(xOuter - 0.05f, 2.1f, 1.75f);
  glVertex3f(xInner + 0.05f, 0.0f, 1.75f);
  glVertex3f(xInner + 0.05f, 2.1f, 1.75f);
  glEnd();
  solidBox(xOuter - 0.12f, 0.0f, 1.70f, xOuter + 0.02f, 0.10f, 1.85f, 1.0f,
           1.0f, 1.0f, 0.9f, 0.9f, 0.9f, 1.0f, 1.0f, 1.0f);
  solidBox(xInner - 0.02f, 0.0f, 1.70f, xInner + 0.12f, 0.10f, 1.85f, 1.0f,
           1.0f, 1.0f, 0.9f, 0.9f, 0.9f, 1.0f, 1.0f, 1.0f);
}

/**
 * @brief Renders the foundational plinth of the building
 */
void renderBuildingPlinth(float FX1, float FX2) {
  // Concrete base (Plinth)
  solidBox(FX1, 0.0f, -0.7f, FX2, 0.15f, 1.8f, 0.55f, 0.55f, 0.53f, 0.5f, 0.5f,
           0.48f, 0.65f, 0.65f, 0.63f);
}

/**
 * @brief Renders the main exterior walls and structure
 */
void renderMainWalls(float BX1, float BX2) {
  // Bottom Section (Grey Concrete)
  solidBox(BX1, 0, -0.5f, BX2, 0.35f, 0, 0.35f, 0.38f, 0.42f, 0.3f, 0.33f,
           0.37f, 0.45f, 0.48f, 0.52f);
  // Main Wall (White/Cream)
  solidBox(BX1, 0.35f, -0.5f, BX2, 1.0f, 0, 0.95f, 0.95f, 0.92f, 0.85f, 0.85f,
           0.82f, 1.0f, 1.0f, 0.98f);
  // Back Wall
  solidBox(BX1, 0, -0.55f, BX2, 1.0f, -0.5f, 0.90f, 0.90f, 0.87f, 0.8f, 0.8f,
           0.77f, 0.95f, 0.95f, 0.92f);
  // Left Side Wall
  solidBox(BX1 - 0.05f, 0, -0.55f, BX1, 1.0f, 0, 0.85f, 0.85f, 0.82f, 0.75f,
           0.75f, 0.72f, 0.9f, 0.9f, 0.88f);
  // Right Side Wall
  solidBox(BX2, 0, -0.55f, BX2 + 0.05f, 1.0f, 0, 0.85f, 0.85f, 0.82f, 0.75f,
           0.75f, 0.72f, 0.9f, 0.9f, 0.88f);
}

/**
 * @brief Renders the vertical structural columns
 */
void renderStructuralColumns(float BX1, float BX2) {
  float cw = 0.12f;
  float colX[] = {BX1, (BX1 + BX2) * 0.5f, BX2};
  for (int i = 0; i < 3; i++) {
    solidBox(colX[i] - cw, 0, 0, colX[i] + cw, 1.0f, 0.12f, 0.85f, 0.85f, 0.82f,
             0.75f, 0.75f, 0.72f, 0.95f, 0.95f, 0.92f);
  }
}

/**
 * @brief Renders the windows and glass panels
 */
void renderBuildingWindows(float BX1, float BX2) {
  float winW = 0.45f, winH = 0.35f;
  float winY = 0.55f;
  float winX[] = {BX1 + 1.2f, BX1 + 2.8f, BX2 - 2.8f, BX2 - 1.2f};
  for (int i = 0; i < 4; i++) {
    // Window Frame
    solidBox(winX[i] - winW - 0.02f, winY - 0.02f, 0.01f,
             winX[i] + winW + 0.02f, winY + winH + 0.02f, 0.03f, 0.4f, 0.4f,
             0.4f, 0.3f, 0.3f, 0.3f, 0.5f, 0.5f, 0.5f);
    // Glass
    glColor3f(0.1f, 0.15f, 0.25f);
    glBegin(GL_QUADS);
    glVertex3f(winX[i] - winW, winY, 0.031f);
    glVertex3f(winX[i] + winW, winY, 0.031f);
    glVertex3f(winX[i] + winW, winY + winH, 0.031f);
    glVertex3f(winX[i] - winW, winY + winH, 0.031f);
    glEnd();
  }
}

/**
 * @brief Renders the building's roof structure
 */
void renderRoofSystem(float BX1, float BX2) {
  // Main Roof Slab
  solidBox(BX1 - 0.3f, 1.0f, -0.7f, BX2 + 0.3f, 1.12f, 0.25f, 0.85f, 0.85f,
           0.82f, 0.75f, 0.75f, 0.72f, 0.95f, 0.95f, 0.92f);

  // Decorative Roof Edge (Top)
  float rw = 0.08f;
  solidBox(BX1 - 0.2f, 1.12f, -0.6f, BX2 + 0.2f, 1.35f, -0.55f, 0.85f, 0.82f,
           0.78f, 0.75f, 0.72f, 0.68f, 0.95f, 0.92f, 0.88f);

  // Ventilation Vents (Top)
  for (float vx = BX1 + 0.5f; vx < BX2 - 0.5f; vx += 0.8f) {
    for (float vy = 1.15f; vy < 1.30f; vy += 0.08f) {
      glColor3f(0.2f, 0.2f, 0.2f);
      glBegin(GL_QUADS);
      glVertex3f(vx, vy, -0.54f);
      glVertex3f(vx + 0.4f, vy, -0.54f);
      glVertex3f(vx + 0.4f, vy + 0.04f, -0.54f);
      glVertex3f(vx, vy + 0.04f, -0.54f);
      glEnd();
    }
  }
}

/**
 * @brief Top-level function to assemble the building structure
 */
void drawLongWindow(float x, float y, float z) {
  float w = 0.85f; // width between doors is 1.7, so 0.85 is a good width
  float h = 0.35f;

  // Window Frame (Dark grey)
  glColor3f(0.35f, 0.35f, 0.38f);
  solidBox(x - w / 2 - 0.02f, y - 0.02f, z, x + w / 2 + 0.02f, y + h + 0.02f,
           z + 0.02f, 0.35f, 0.35f, 0.38f, 0.3f, 0.3f, 0.33f, 0.4f, 0.4f,
           0.43f);

  // Glass panes (Dark reflective grey)
  glColor3f(0.15f, 0.18f, 0.22f);
  for (int i = 0; i < 3; i++) {
    float px = x - w / 2 + (i * w / 3);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(px + 0.01f, y + 0.01f, z + 0.021f);
    glVertex3f(px + w / 3 - 0.01f, y + 0.01f, z + 0.021f);
    glVertex3f(px + w / 3 - 0.01f, y + h - 0.01f, z + 0.021f);
    glVertex3f(px + 0.01f, y + h - 0.01f, z + 0.021f);
    glEnd();
  }
}

void drawBuilding(bool withWindows = false, bool isMainBuilding = true) {
  float BX1 = -4.5f, BX2 = 4.5f; // building X extents
  float BZ1 = -0.7f, BZ2 = 0.7f; // building depth
  float BZ_Back = -3.5f;
  float FX1 = BX1 - 1.25f;
  float FX2 = BX2 + 1.25f;

  //           0. FOUNDATION PLINTH (The base the whole building sits on)
  // 1. Wide Front Part (Under stairs and main front)
  solidBox(FX1, 0.0f, -0.7f, FX2, 0.15f, 1.8f, 0.6f, 0.62f, 0.65f, 0.6f, 0.62f,
           0.65f, 0.92f, 0.60f, 0.38f);
  // 2. Narrower Back Part (Under rear sloping section)
  solidBox(BX1, 0.0f, BZ_Back - 0.2f, BX2, 0.15f, -0.7f, 0.6f, 0.62f, 0.65f,
           0.6f, 0.62f, 0.65f, 0.92f, 0.60f, 0.38f);

  // ── CONCRETE ACCESS RAMP (shape depends on which building) ──
  glDisable(GL_LIGHTING);
  if (isMainBuilding) {
    // FRONT RAMP: slopes straight out from the building's front face (along Z)
    // Plinth edge z=1.8 (y=0.15) → ground z=3.5 (y=0), X range: 4.6–5.4

    // Top sloped surface
    glColor3f(0.55f, 0.52f, 0.48f);
    glBegin(GL_QUADS);
    glVertex3f(4.6f, 0.15f, 1.8f); // back-left  (plinth level)
    glVertex3f(5.4f, 0.15f, 1.8f); // back-right (plinth level)
    glVertex3f(5.4f, 0.00f, 3.5f); // front-right (ground)
    glVertex3f(4.6f, 0.00f, 3.5f); // front-left  (ground)
    glEnd();
    // Front lip
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
  } else {
    // SIDE RAMP: slopes sideways along X on the RIGHT wall of the left building,
    // facing toward the path between the two buildings.
    // Plinth edge x=5.70 (y=0.15) → ground x=7.20 (y=0), Z range: 0.8–1.8

    // Top sloped surface
    glColor3f(0.55f, 0.52f, 0.48f);
    glBegin(GL_QUADS);
    glVertex3f(5.70f, 0.15f, 0.8f);  // back-left  (plinth level)
    glVertex3f(5.70f, 0.15f, 1.8f);  // front-left (plinth level)
    glVertex3f(7.20f, 0.00f, 1.8f);  // front-right (ground)
    glVertex3f(7.20f, 0.00f, 0.8f);  // back-right  (ground)
    glEnd();
    // Front face (triangle at z=1.8)
    glColor3f(0.48f, 0.45f, 0.42f);
    glBegin(GL_TRIANGLES);
    glVertex3f(5.70f, 0.15f, 1.8f);
    glVertex3f(5.70f, 0.00f, 1.8f);
    glVertex3f(7.20f, 0.00f, 1.8f);
    glEnd();
    // Back face (triangle at z=0.8)
    glColor3f(0.48f, 0.45f, 0.42f);
    glBegin(GL_TRIANGLES);
    glVertex3f(5.70f, 0.15f, 0.8f);
    glVertex3f(7.20f, 0.00f, 0.8f);
    glVertex3f(5.70f, 0.00f, 0.8f);
    glEnd();
    // Left vertical wall (at x=5.70, flush with plinth edge)
    glColor3f(0.50f, 0.47f, 0.44f);
    glBegin(GL_QUADS);
    glVertex3f(5.70f, 0.00f, 0.8f);
    glVertex3f(5.70f, 0.15f, 0.8f);
    glVertex3f(5.70f, 0.15f, 1.8f);
    glVertex3f(5.70f, 0.00f, 1.8f);
    glEnd();
  }
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

  // door and window positions (5 each, perfectly spaced)
  float dX[5] = {-3.15f, -1.35f, 0.45f, 2.25f, 4.05f};
  float winX[5] = {-4.05f, -2.25f, -0.45f, 1.35f, 3.15f};
  float VY1 = 2.65f,
        VY2 = 3.25f; // Declared here to be used in First Floor and Attic

  //           1. GROUND FLOOR (Striped Front & Sides)
  // Bottom Band (Grey)
  solidBox(BX1, 0.0f, BZ1, BX2, 0.5f, BZ2, 0.35f, 0.38f, 0.41f, 0.35f, 0.38f,
           0.41f, 0.35f, 0.38f, 0.41f);
  // Top Band (White Front, Orange Side)
  solidBox(BX1, 0.5f, BZ1, BX2, 1.0f, BZ2, 0.92f, 0.92f, 0.92f, 0.92f, 0.60f,
           0.38f, 0.92f, 0.60f, 0.38f);

  // Detailed doors and windows
  for (int i = 0; i < 5; i++) {
    float angle = 0.0f;
    // Far-right door (index 4) opens when we transition
    if (i == 4 && currentMode != MODE_EXTERIOR) {
      angle = targetDoorAngle;
    }
    drawDetailedDoor(dX[i], 0.0f, BZ2 + 0.002f, angle);

    // Draw windows (5 windows total)
    if (withWindows) {
      drawLongWindow(winX[i], 0.55f, BZ2 + 0.001f);
    }
  }

  //           2. FIRST FLOOR (Striped Front, Orange Side)
  // Front face (Two-tone: Bottom Grey, Top White)
  // Bottom Half (Grey) - just above balcony
  glColor3f(0.35f, 0.38f, 0.41f);
  glBegin(GL_QUADS);
  glVertex3f(BX1, 1.0f, BZ2 + 0.001f);
  glVertex3f(BX2, 1.0f, BZ2 + 0.001f);
  glVertex3f(BX2, 1.5f, BZ2 + 0.001f);
  glVertex3f(BX1, 1.5f, BZ2 + 0.001f);
  glEnd();
  // Top Half (White)
  glColor3f(0.92f, 0.92f, 0.92f);
  glBegin(GL_QUADS);
  glVertex3f(BX1, 1.5f, BZ2 + 0.001f);
  glVertex3f(BX2, 1.5f, BZ2 + 0.001f);
  glVertex3f(BX2, VY1, BZ2 + 0.001f);
  glVertex3f(BX1, VY1, BZ2 + 0.001f); // Extended to VY1
  glEnd();
  // Solid Orange Sides & Back
  solidBox(BX1, 1.0f, BZ1, BX2, VY1, BZ2, 0.35f, 0.38f, 0.41f, 0.88f, 0.72f,
           0.52f, 0.82f, 0.66f, 0.46f);

  // Detailed doors and windows (Balcony level)
  for (int i = 0; i < 5; i++) {
    drawDetailedDoor(dX[i], 1.02f, BZ2 + 0.002f);

    // Draw windows (5 windows total)
    if (withWindows) {
      drawLongWindow(winX[i], 1.55f, BZ2 + 0.001f);
    }
  }

  //           3. UPPER ATTIC (Front Face & Sides)
  float VX1 = BX1, VX2 = BX2;
  float VZ1 = -0.5f, VZ2 = 0.55f;
  solidBox(VX1, VY1, VZ1, VX2, VY2, VZ2, 0.88f, 0.72f, 0.52f, 0.88f, 0.72f,
           0.52f, 0.82f, 0.66f, 0.46f);

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
  glVertex3f(BX1 - 0.1f, VY2, VZ1);
  glVertex3f(BX2 + 0.1f, VY2, VZ1);
  glVertex3f(BX2 + 0.1f, BY_Back, BZ_Back);
  glVertex3f(BX1 - 0.1f, BY_Back, BZ_Back);
  glEnd();

  // Rear Trapezoidal Side Walls (Orange-Tan)
  glColor3f(0.88f, 0.72f, 0.52f);
  // Left Side
  glBegin(GL_QUADS);
  glVertex3f(BX1, 0.0f, VZ1);
  glVertex3f(BX1, 0.0f, BZ_Back);
  glVertex3f(BX1, BY_Back, BZ_Back);
  glVertex3f(BX1, VY2, VZ1);
  glEnd();
  // Right Side
  glBegin(GL_QUADS);
  glVertex3f(BX2, 0.0f, VZ1);
  glVertex3f(BX2, 0.0f, BZ_Back);
  glVertex3f(BX2, BY_Back, BZ_Back);
  glVertex3f(BX2, VY2, VZ1);
  glEnd();
  // Back Wall (Pinkish-Tan / Peach)
  glColor3f(0.95f, 0.76f, 0.68f);
  glBegin(GL_QUADS);
  glVertex3f(BX1, 0.0f, BZ_Back);
  glVertex3f(BX2, 0.0f, BZ_Back);
  glVertex3f(BX2, BY_Back, BZ_Back);
  glVertex3f(BX1, BY_Back, BZ_Back);
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
    solidBox(colX - 0.08f, 0.0f, BZ_Back - 0.08f, colX + 0.08f, BY_Back,
             BZ_Back + 0.05f, 0.32f, 0.32f, 0.35f, 0.28f, 0.28f, 0.31f, 0.35f,
             0.35f, 0.38f);

    // Vertical Drain Pipe (White)
    glColor3f(0.95f, 0.95f, 0.95f);
    solidBox(colX + 0.06f, 0.0f, BZ_Back - 0.09f, colX + 0.10f, BY_Back + 0.05f,
             BZ_Back - 0.03f, 0.95f, 0.95f, 0.95f, 0.9f, 0.9f, 0.9f, 0.95f,
             0.95f, 0.95f);

    // Ceiling Overhang
    solidBox(colX + 0.1f, BY_Back - 0.05f, BZ_Back - 0.08f,
             colX + segWidth - 0.1f, BY_Back, BZ_Back, 0.95f, 0.95f, 0.95f,
             0.9f, 0.9f, 0.9f, 0.95f, 0.95f, 0.95f);

    // Final Closing Column at the very end
    if (s == 4) {
      float lastX = colX + segWidth;
      glColor3f(0.32f, 0.32f, 0.35f);
      solidBox(lastX - 0.08f, 0.0f, BZ_Back - 0.08f, lastX + 0.08f, BY_Back,
               BZ_Back + 0.05f, 0.32f, 0.32f, 0.35f, 0.28f, 0.28f, 0.31f, 0.35f,
               0.35f, 0.38f);
    }

    // Function to draw each window in the segment
    auto drawSegmentWindow = [&](float x, float y, bool vertical) {
      // Window Sill (Smaller)
      solidBox(x - 0.58f, y - 0.02f, BZ_Back - 0.04f, x + 0.58f, y + 0.02f,
               BZ_Back, 0.9f, 0.9f, 0.9f, 0.8f, 0.8f, 0.8f, 0.95f, 0.95f,
               0.95f);

      // Multi-Pane Glass (With Specular Highlight)
      float pw = 1.1f / 3.0f;
      for (int i = 0; i < 3; i++) {
        float px = x - 0.55f + (i * pw);
        if ((i + s) % 2 == 0)
          glColor3f(0.2f, 0.3f, 0.4f);
        else
          glColor3f(0.55f, 0.65f, 0.75f);

        glBegin(GL_QUADS);
        glNormal3f(0, 0, -1);
        glVertex3f(px + 0.01f, y + 0.03f, BZ_Back - 0.01f);
        glVertex3f(px + pw - 0.01f, y + 0.03f, BZ_Back - 0.01f);
        glVertex3f(px + pw - 0.01f, y + 0.38f, BZ_Back - 0.01f);
        glVertex3f(px + 0.01f, y + 0.38f, BZ_Back - 0.01f);
        glEnd();
      }

      // Grills (Tightly aligned with smaller glass)
      glColor3f(0.1f, 0.1f, 0.1f);
      glLineWidth(1.2f);
      glBegin(GL_LINES);
      if (vertical) {
        for (float i = -0.54f; i <= 0.54f; i += 0.08f) {
          glVertex3f(x + i, y + 0.03f, BZ_Back - 0.015f);
          glVertex3f(x + i, y + 0.40f, BZ_Back - 0.015f);
        }
      } else {
        for (float i = 0.10f; i <= 0.35f; i += 0.12f) {
          glVertex3f(x - 0.55f, y + i, BZ_Back - 0.015f);
          glVertex3f(x + 0.55f, y + i, BZ_Back - 0.015f);
        }
      }
      glEnd();
    };

    drawSegmentWindow(segX, 0.15f, true);  // Ground floor
    drawSegmentWindow(segX, 0.95f, false); // First floor
  }
  // Final closing column on the right
  solidBox(BX2 - 0.35f, 0.0f, BZ_Back - 0.01f, BX2, BY_Back, BZ_Back + 0.05f,
           0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f);

  //           2. SECOND FLOOR BALCONY
  solidBox(BX1, 1.0f, 0.7f, BX2, 1.08f, 1.65f, 0.72f, 0.72f, 0.72f, 0.58f,
           0.58f, 0.58f, 0.66f, 0.66f, 0.66f);

  // --- 2b. BALCONY RAILINGS ---
  float bDensity = 0.06f;
  float bRSize = 0.015f;
  glColor3f(0.9f, 0.9f, 0.9f);
  // Front railing
  for (float x = BX1; x <= BX2; x += bDensity) {
    solidBox(x, 1.08f, 1.63f, x + bRSize, 1.58f, 1.63f + bRSize, 0.92f, 0.92f,
             0.92f, 0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f);
  }
  // Top Bar (Front only)
  solidBox(BX1, 1.55f, 1.63f, BX2, 1.58f, 1.63f + bRSize, 0.8f, 0.8f, 0.8f,
           0.7f, 0.7f, 0.7f, 0.85f, 0.85f, 0.85f);

  //           4b. SIDE WALL CONCRETE GRID (The "Cross" pattern)
  glColor3f(0.75f, 0.65f, 0.55f);
  // Horizontal beams
  solidBox(BX1 - 0.01f, 1.0f, BZ_Back, BX1 + 0.05f, 1.15f, 0.7f, 0.7f, 0.6f,
           0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);
  solidBox(BX2 - 0.05f, 1.0f, BZ_Back, BX2 + 0.01f, 1.15f, 0.7f, 0.7f, 0.6f,
           0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);
  // Vertical beams
  solidBox(BX1 - 0.01f, 0.0f, -2.0f, BX1 + 0.05f, 2.0f, -1.85f, 0.7f, 0.6f,
           0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);
  solidBox(BX2 - 0.05f, 0.0f, -2.0f, BX2 + 0.01f, 2.0f, -1.85f, 0.7f, 0.6f,
           0.5f, 0.75f, 0.65f, 0.55f, 0.75f, 0.65f, 0.55f);

  //           4. MAIN SLANTED CORRUGATED ROOF
  float RX1 = BX1, RX2 = BX2;
  float RY_Back = 2.75f,
        RY_Front = 2.40f; // Raised to create the vertical gap above the canopy
  float RZ_Back = -0.5f, RZ_Front = 2.0f;

  // Corrugated stripes (slanted)
  bool dark = false;
  for (float x = RX1; x < RX2; x += 0.12f) {
    float x_next = x + 0.12f;
    if (x_next > RX2)
      x_next = RX2;
    glColor3f(dark ? 0.65f : 0.75f, dark ? 0.65f : 0.75f, dark ? 0.68f : 0.78f);
    dark = !dark;
    glBegin(GL_QUADS);
    glVertex3f(x, RY_Back, RZ_Back);
    glVertex3f(x_next, RY_Back, RZ_Back);
    glVertex3f(x_next, RY_Front, RZ_Front);
    glVertex3f(x, RY_Front, RZ_Front);
    glEnd();
  }

  //           5. STAIRCASE HORIZONTAL GABLE CANOPIES with TRUSSES
  // Ridge runs horizontally (parallel to building front)
  float CY_P = 2.15f; // Ridge peak
  float CY_E = 1.70f; // Eave height (Restored)
  float CZ_B = -0.5f, CZ_F = 2.0f;
  float midZ = (CZ_B + CZ_F) * 0.5f;

  auto drawHorizontalGable = [&](float xOut, float xIn) {
    bool dark = false;
    for (float x = xOut; x < xIn; x += 0.12f) {
      float x_next = x + 0.12f;
      if (x_next > xIn)
        x_next = xIn;
      glColor3f(dark ? 0.65f : 0.75f, dark ? 0.65f : 0.75f,
                dark ? 0.68f : 0.78f);
      dark = !dark;
      // Back slope
      glBegin(GL_QUADS);
      glVertex3f(x, CY_E, CZ_B);
      glVertex3f(x_next, CY_E, CZ_B);
      glVertex3f(x_next, CY_P, midZ);
      glVertex3f(x, CY_P, midZ);
      glEnd();
      // Front slope
      glBegin(GL_QUADS);
      glVertex3f(x, CY_P, midZ);
      glVertex3f(x_next, CY_P, midZ);
      glVertex3f(x_next, CY_E, CZ_F);
      glVertex3f(x, CY_E, CZ_F);
      glEnd();
    }
    // Metal Edge Capping
    glColor3f(0.6f, 0.62f, 0.65f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    glVertex3f(xOut, CY_E, CZ_F + 0.01f);
    glVertex3f(xOut, CY_P, midZ);
    glVertex3f(xOut, CY_E, CZ_B - 0.01f);
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
      glVertex3f(x, CY_E, CZ_B);
      glVertex3f(x, CY_E, CZ_F);
      glVertex3f(x, CY_E, CZ_B);
      glVertex3f(x, CY_P, midZ);
      glVertex3f(x, CY_E, CZ_F);
      glVertex3f(x, CY_P, midZ);
      // Diagonal bracing
      glVertex3f(x, CY_E, (CZ_B + midZ) * 0.5f);
      glVertex3f(x, CY_P, midZ);
      glVertex3f(x, CY_E, (CZ_F + midZ) * 0.5f);
      glVertex3f(x, CY_P, midZ);
      glEnd();
    }
  };
  drawHTruss(-6.0f, -4.5f);
  drawHTruss(4.5f, 6.0f);

  //           6. SUPPORT PILLARS (Ending at roof level)
  glColor3f(0.55f, 0.56f, 0.58f); // Weathered Concrete Grey
  glLineWidth(4.0f);
  // Main front pillars
  float pX[6] = {-4.5f, -2.7f, -0.9f, 0.9f, 2.7f, 4.5f};
  for (int i = 0; i < 6; i++) {
    glBegin(GL_LINES);
    glVertex3f(pX[i], 0.0f, 1.63f);
    glVertex3f(pX[i], 2.45f,
               1.63f); // Corrected height to meet the new higher roof
    glEnd();
  }
  // Outer staircase pillars (Restored to 1.70f height)
  glBegin(GL_LINES);
  // Front pillars
  glVertex3f(-4.55f, 0.0f, 1.9f);
  glVertex3f(-4.55f, CY_E, 1.9f);
  glVertex3f(4.55f, 0.0f, 1.9f);
  glVertex3f(4.55f, CY_E, 1.9f);
  glVertex3f(-5.95f, 0.0f, 1.9f);
  glVertex3f(-5.95f, CY_E, 1.9f);
  glVertex3f(5.95f, 0.0f, 1.9f);
  glVertex3f(5.95f, CY_E, 1.9f);
  // Back pillars
  glVertex3f(-4.55f, 0.0f, -0.4f);
  glVertex3f(-4.55f, CY_E, -0.4f);
  glVertex3f(4.55f, 0.0f, -0.4f);
  glVertex3f(4.55f, CY_E, -0.4f);
  glVertex3f(-5.95f, 0.0f, -0.4f);
  glVertex3f(-5.95f, CY_E, -0.4f);
  glVertex3f(5.95f, 0.0f, -0.4f);
  glVertex3f(5.95f, CY_E, -0.4f);
  glEnd();
  glLineWidth(1.0f);

  //           7. STAIRS (U-shaped / Dog-leg)
  drawStaircase(-5.7f, -4.5f, 0.7f, -0.3f); // One step back
  drawStaircase(5.7f, 4.5f, 0.7f, -0.3f);

  // Draw students on the balcony
  if (currentMode != MODE_MORNING_CINEMATIC) {
    drawStudents();
  }

  if (currentMode == MODE_TRANSITION || currentMode == MODE_TRANSITION_BACK) {
    drawClassroomInterior();
  }

  glPopMatrix(); // End of vertical shift
}

//           display
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  // ULTIMATE LOCKDOWN: Apply clamps BEFORE any rendering math
  if (currentMode == MODE_CLASSROOM) {
    if (posX < -2.8f)
      posX = -2.8f;
    if (posX > 2.8f)
      posX = 2.8f;
    if (posZ < -6.3f)
      posZ = -6.3f;
    if (posZ > 0.45f)
      posZ = 0.45f;
    if (camHeight < 0.4f)
      camHeight = 0.4f;
    if (camHeight > 1.8f)
      camHeight = 1.8f; // Human height cap
    zoom = 0.0f;
  }

  float p = transitionProgress;
  float easeP = p * p * (3.0f - 2.0f * p); // smooth step interpolation

  float currentCamHeight = camHeight;
  float currentZoom = zoom;
  float currentTilt = 12.0f;

  if (currentMode == MODE_TRANSITION || currentMode == MODE_TRANSITION_BACK) {
    float targetCamHeight = 0.6f;
    // Watch from outside the door instead of following closely
    float targetZoom = 4.0f;
    float targetTilt = 0.0f;

    currentCamHeight = camHeight * (1.0f - easeP) + targetCamHeight * easeP;
    currentZoom = zoom * (1.0f - easeP) + targetZoom * easeP;
    currentTilt = 12.0f * (1.0f - easeP) + targetTilt * easeP;

    glTranslatef(0.0f, -currentCamHeight, -currentZoom);
    glRotatef(currentTilt, 1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glRotatef(autoAngle, 0.0f, 1.0f, 0.0f);
  } else if (currentMode == MODE_CLASSROOM || (currentMode == MODE_MORNING_CINEMATIC && morningProgress >= 0.50f)) {
    if (currentMode == MODE_MORNING_CINEMATIC) {
        // Synced timeline for lead student (Student 1) walking from 0.50f to 0.65f
        float p1 = (morningProgress - 0.50f) / 0.15f;
        if (p1 < 0.0f) p1 = 0.0f;
        if (p1 > 1.0f) p1 = 1.0f;

        auto getActualLeadPos = [](float p, float targetX, float targetZ, float& x, float& z, float& angle) {
            if (p < 0.0f) p = 0.0f;
            if (p > 1.0f) p = 1.0f;
            
            float aisleZ = targetZ + 0.3f; // Pathway immediately behind the target chair

            // Phase 1 (0.0 to 0.6): Walk down left aisle (X = -2.6) from Z = -7.5 to aisleZ
            if (p < 0.6f) {
                float t = p / 0.6f;
                x = -2.6f;
                z = -7.5f + t * (aisleZ - (-7.5f));
                angle = 0.0f;
            }
            // Phase 2 (0.6 to 0.9): Walk horizontally along the row aisle from X = -2.6 to targetX at aisleZ
            else if (p < 0.9f) {
                float t = (p - 0.6f) / 0.3f;
                x = -2.6f + t * (targetX - (-2.6f));
                z = aisleZ;
                angle = 90.0f;
            }
            // Phase 3 (0.9 to 1.0): Step forward into the chair (from aisleZ to targetZ)
            else {
                float t = (p - 0.9f) / 0.1f;
                x = targetX;
                z = aisleZ - t * (aisleZ - targetZ);
                angle = 90.0f + (t * 90.0f); // turn to 180 (facing whiteboard)
            }
        };
        float lx1, lz1, lang1; getActualLeadPos(p1, 1.0f, 0.2f, lx1, lz1, lang1);

        if (morningProgress < 0.65f) {
            // Step 1: Follow the lead student (Student 1) closely from behind until they sit
            float camX, camZ, camRot;
            if (p1 < 0.6f) {
                camX = -2.6f;
                camZ = lz1 - 1.0f;
                if (camZ < -7.3f) camZ = -7.3f;
                camRot = 180.0f;
            } else if (p1 < 0.9f) {
                float t = (p1 - 0.6f) / 0.3f;
                camX = lx1 - 1.0f;
                if (camX < -2.6f) camX = -2.6f;
                camZ = 0.5f;
                camRot = 180.0f + (t * 90.0f);
            } else {
                float t = (p1 - 0.9f) / 0.1f;
                camX = 1.0f;
                camZ = 0.5f - (t * 1.0f);
                if (camZ < -0.5f) camZ = -0.5f;
                camRot = 270.0f; // STAY looking from the side while the student turns and sits!
            }

            glRotatef(camRot, 0.0f, 1.0f, 0.0f);
            glTranslatef(-camX, -1.2f, -camZ);
        } else if (morningProgress < 0.70f) {
            // Step 2: Camera reaches seat and turns smoothly from 270° to 360° (0°) to face the whiteboard screen/blackboard
            float t_turn = (morningProgress - 0.65f) / 0.05f;
            float easeP = t_turn * t_turn * (3.0f - 2.0f * t_turn); // smooth step
            float camRot = 270.0f * (1.0f - easeP) + 360.0f * easeP;

            glRotatef(camRot, 0.0f, 1.0f, 0.0f);
            glTranslatef(-1.0f, -1.2f, -(-0.5f));
        } else {
            // Step 3: Camera stays static, beautifully framing the screen view while Student 3, Student 4, and the Teacher walk in one-by-one!
            glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(-1.0f, -1.2f, -(-0.5f));
        }
    } else {
        glRotatef(autoAngle, 0.0f, 1.0f, 0.0f);
        glTranslatef(-posX, -currentCamHeight, -posZ);
    }
    glPushMatrix();
  } else if (currentMode == MODE_MORNING_CINEMATIC) {
    if (morningProgress < 0.25f) {
      // Phase 1: Sunrise - Fixed Vantage Point
      currentCamHeight = 3.4f;
      currentZoom = 25.0f; // Pulled way back for a wide shot
      currentTilt = -59.0f;
      autoAngle = 30.0f; // Look left to center between buildings
      glTranslatef(-7.0f, -currentCamHeight, -currentZoom); // Camera placed at X=7
      glRotatef(currentTilt, 0.0f, 1.0f, 0.0f);
      glPushMatrix();
      glRotatef(autoAngle, 0.0f, 1.0f, 0.0f);
    } else {
      // Phase 2: Students walk outside - Camera swoops down and follows them from behind!
      float p = (morningProgress - 0.25f) / 0.25f;
      float easeP = p * p * (3.0f - 2.0f * p); // smooth step transition from wide overview
      
      currentCamHeight = 3.4f * (1.0f - easeP) + 1.2f * easeP;
      currentZoom = 25.0f * (1.0f - easeP) + 5.0f * easeP;
      
      float tiltX = 0.0f * (1.0f - easeP) + 10.0f * easeP;
      float yawY1 = -59.0f * (1.0f - easeP) + 0.0f * easeP;
      float yawY2 = 30.0f * (1.0f - easeP) + 0.0f * easeP;
      
      float trackX = 7.0f * (1.0f - easeP) + actStudentX * easeP;
      float trackZ = 0.0f * (1.0f - easeP) + actStudentZ * easeP;

      glTranslatef(-trackX, -currentCamHeight, -(trackZ + currentZoom));
      glRotatef(tiltX, 1.0f, 0.0f, 0.0f);
      glRotatef(yawY1, 0.0f, 1.0f, 0.0f);
      glPushMatrix();
      glRotatef(yawY2, 0.0f, 1.0f, 0.0f);
    }
  } else {
    glTranslatef(0.0f, -currentCamHeight, -currentZoom);
    glRotatef(12.0f, 1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glRotatef(autoAngle, 0.0f, 1.0f, 0.0f);
  }

  if (currentMode == MODE_CLASSROOM || (currentMode == MODE_MORNING_CINEMATIC && morningProgress >= 0.50f)) {
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    drawClassroomInterior();
    if (currentMode == MODE_MORNING_CINEMATIC) {
         // Synced individual timeline progress variables:
         float p1 = (morningProgress - 0.50f) / 0.15f;
         if (p1 < 0.0f) p1 = 0.0f;
         if (p1 > 1.0f) p1 = 1.0f;

         float p_teacher = 0.0f;
         if (morningProgress >= 0.90f) {
             p_teacher = (morningProgress - 0.90f) / 0.07f;
             if (p_teacher > 1.0f) p_teacher = 1.0f;
         }

         auto getInteriorPos = [](float p, float targetX, float targetZ, float& x, float& z, float& angle) {
              if (p < 0.0f) p = 0.0f;
              if (p > 1.0f) p = 1.0f;
              
              float aisleZ = targetZ + 0.3f;
              
              if (p < 0.6f) {
                  float t = p / 0.6f;
                  x = -2.6f;
                  z = -7.5f + t * (aisleZ - (-7.5f));
                  angle = 0.0f;
              }
              else if (p < 0.9f) {
                  float t = (p - 0.6f) / 0.3f;
                  x = -2.6f + t * (targetX - (-2.6f));
                  z = aisleZ;
                  angle = 90.0f;
              }
              else {
                  float t = (p - 0.9f) / 0.1f;
                  x = targetX;
                  z = aisleZ - t * (aisleZ - targetZ);
                  angle = 90.0f + (t * 90.0f);
              }
         };
         
         float x1, z1, a1; 
         getInteriorPos(p1, 1.0f, 0.2f, x1, z1, a1);
         bool seat1 = (p1 >= 0.95f);

         if (p1 > 0.0f) drawStudent(x1, 0.0f, z1, a1, morningProgress * 100.0f, seat1, 1, false);

         // Generate the remaining 34 background seats dynamically
         float seatsX[34];
         float seatsZ[34];
         int sIdx = 0;
         for (int row = 0; row < 7; row++) {
             float dz = -5.5f + row * 0.9f;
             for (int col = 0; col < 5; col++) {
                 float dx = -2.0f + col * 1.0f;
                 float tz = dz + 0.2f;
                 float tx = dx;
                 // Skip main student's seat (X=1.0, Z=0.2)
                 if (std::abs(tx - 1.0f) < 0.1f && std::abs(tz - 0.2f) < 0.1f) continue;
                 if (sIdx < 34) {
                     seatsX[sIdx] = tx;
                     seatsZ[sIdx] = tz;
                     sIdx++;
                 }
             }
         }

         // Draw the 34 background students filing in sequentially
         for (int i = 0; i < 34; i++) {
             // Spread their start times between 0.70f and 0.85f so they all enter one-by-one!
             float start_p = 0.70f + (i / 33.0f) * 0.15f;
             float walk_dur = 0.12f; // SLOWER, elegant walk to keep the sequence dynamic
             float p_i = (morningProgress - start_p) / walk_dur;
             
             if (p_i > 0.0f) {
                 float sx, sz, sa;
                 getInteriorPos(p_i, seatsX[i], seatsZ[i], sx, sz, sa);
                 bool s_seat = (p_i >= 0.95f);
                 // Vary walk animation timing slightly so they look natural
                 float anim = morningProgress * (100.0f + (i % 5) * 15.0f);
                 // Assign a random-looking uniform (colors 1 through 6)
                 int uniformType = (i % 6) + 1; 
                 drawStudent(sx, 0.0f, sz, sa, anim, s_seat, uniformType, false);
             }
         }

         if (morningProgress >= 0.90f) {
             float tx = -2.5f + (p_teacher * 2.0f);
             float tz = -7.5f + (p_teacher * 0.8f);
             float ta = (p_teacher < 0.90f) ? 45.0f : 0.0f;
             // Variant 99 forces the pure black suit for the teacher!
             drawStudent(tx, 0.0f, tz, ta, morningProgress * 200.0f, false, 99, isTeacherTeaching);
         }
    }
    glPopMatrix();
  } else {
    drawSky();
    drawEarth();
    // Helper lambda to wrap car progress smoothly across the boundaries
    auto wrapProgress = [](float progress, float offset) {
      float p = progress + offset;
      while (p > 50.0f)
        p -= 100.0f;
      while (p < -50.0f)
        p += 100.0f;
      return p;
    };

    // Draw multi-lane, multi-car traffic with only black color for all car
    // types!
    if (currentMode != MODE_MORNING_CINEMATIC) {
      drawCar(wrapProgress(carProgress, 0.0f), 0.0f, 13.0f, 90.0f, 0, 0.1f, 0.1f,
              0.1f); // Black Sedan (Lane 1, L to R)
      drawCar(wrapProgress(carProgress, -35.0f), 0.0f, 13.2f, 90.0f, 2, 0.1f,
              0.1f, 0.1f); // Black Pickup (Lane 1, L to R)
      drawCar(wrapProgress(-carProgress, -35.0f), 0.0f, 15.8f, -90.0f, 3, 0.1f,
              0.1f, 0.1f); // Black Sports Car (Lane 2, R to L)
      drawCar(wrapProgress(-carProgress, -18.0f), 0.0f, 16.1f, -90.0f, 4, 0.1f,
              0.4f, 0.9f); // Blue & Yellow Bajaj (Lane 2, R to L)
    }

    if (currentMode == MODE_MORNING_CINEMATIC && morningProgress < 0.50f && morningProgress >= 0.25f) {
        float baseP = (morningProgress - 0.25f) / 0.25f;
         auto getCurvedPos = [](float p, float& x, float& y, float& z, float& angle) {
              if (p < 0.0f) p = 0.0f;
              if (p > 1.0f) p = 1.0f;
              float w0_x = 18.0f, w0_z = 13.5f; // Start
              float w1_x = 9.25f, w1_z = 13.5f; // Sidewalk corner
              float w2_x = 9.25f, w2_z = 3.5f;  // Cobblestone top
              float w3_x = 5.00f, w3_z = 3.5f;  // Ramp base
              float w4_x = 5.00f, w4_z = 1.8f;  // Ramp top (plinth level)
              float w5_x = 4.05f, w5_z = 1.8f;  // Porch door corner
              float w6_x = 4.05f, w6_z = 0.7f;  // Door entrance
              if (p < 0.25f) {
                  float t = p / 0.25f;
                  x = w0_x + t * (w1_x - w0_x);
                  y = 0.0f;
                  z = w0_z;
                  angle = -90.0f;
              } else if (p < 0.55f) {
                  float t = (p - 0.25f) / 0.30f;
                  x = w1_x;
                  y = 0.0f;
                  z = w1_z + t * (w2_z - w1_z);
                  if (t < 0.15f) {
                      float rt = t / 0.15f;
                      angle = -90.0f * (1.0f - rt) + rt * (-180.0f);
                  } else {
                      angle = -180.0f;
                  }
              } else if (p < 0.70f) {
                  float t = (p - 0.55f) / 0.15f;
                  x = w2_x + t * (w3_x - w2_x);
                  y = 0.0f;
                  z = w2_z;
                  if (t < 0.25f) {
                      float rt = t / 0.25f;
                      angle = -180.0f * (1.0f - rt) + rt * (-90.0f);
                  } else {
                      angle = -90.0f;
                  }
              } else if (p < 0.85f) {
                  float t = (p - 0.70f) / 0.15f;
                  x = w3_x;
                  y = t * 0.15f; // Climb up the ramp!
                  z = w3_z + t * (w4_z - w3_z);
                  if (t < 0.25f) {
                      float rt = t / 0.25f;
                      angle = -90.0f * (1.0f - rt) + rt * (-180.0f);
                  } else {
                      angle = -180.0f;
                  }
              } else {
                  float t = (p - 0.85f) / 0.15f;
                  y = 0.15f;
                  if (t < 0.60f) {
                      float t2 = t / 0.60f;
                      x = w4_x + t2 * (w5_x - w4_x);
                      z = w4_z;
                      angle = -90.0f;
                  } else {
                      float t2 = (t - 0.60f) / 0.40f;
                      x = w5_x;
                      z = w5_z + t2 * (w6_z - w5_z);
                      angle = -180.0f;
                  }
              }
         };
         float x1, y1, z1, a1; getCurvedPos(baseP, x1, y1, z1, a1);
         float x3, y3, z3, a3; getCurvedPos(baseP - 0.06f, x3, y3, z3, a3);
         float x4, y4, z4, a4; getCurvedPos(baseP - 0.12f, x4, y4, z4, a4);

         drawStudent(x1, y1, z1, a1, morningProgress * 100.0f, false, 1, false);
         drawStudent(x3, y3, z3, a3, morningProgress * 110.0f, false, 3, false);
         drawStudent(x4, y4, z4, a4, morningProgress * 90.0f, false, 4, false);
    }

    // --- Render Campus Buildings ---

    // 1. Center Building (Main) - WITH WINDOWS
    drawBuilding(true);

    // 2. Left Building (Duplicate) - NO WINDOWS, SIDE RAMP
    glPushMatrix();
    glTranslatef(-15.0f, 0.0f, 0.0f);
    drawBuilding(false, false); // isMainBuilding=false → sideways ramp
    glPopMatrix();
  }

  glPopMatrix();

  // If focusBlurAmount > 0, draw a gorgeous cinematic waking-up vignette/blur lens overlay
  // and full-screen fade mask over the viewport in 2D orthographic projection.
  // This perfectly handles the black startup screen fading out!
  if (currentMode == MODE_MORNING_CINEMATIC && focusBlurAmount > 0.001f) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST); // CRITICAL: Disable depth testing to avoid Z-clipping with 3D buildings!
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 1. FULL PITCH BLACK STARTING OVERLAY (Fades out completely by morningProgress = 0.22)
    // This makes the cinematic start 100% pitch black exactly like the CapCut video!
    float blackScreenAlpha = 0.0f;
    if (morningProgress < 0.22f) {
      float t = morningProgress / 0.22f;
      blackScreenAlpha = 1.0f - (t * t * (3.0f - 2.0f * t)); // Elegant S-curve smoothstep ease-out
    }
    if (blackScreenAlpha > 0.0f) {
      glColor4f(0.0f, 0.0f, 0.0f, blackScreenAlpha);
      glBegin(GL_QUADS);
      glVertex2f(0.0f, 0.0f);
      glVertex2f(800.0f, 0.0f);
      glVertex2f(800.0f, 600.0f);
      glVertex2f(0.0f, 600.0f);
      glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST); // Re-enable depth testing
    glEnable(GL_LIGHTING);
  }

  // Draw Flat 2D "Good Morning" Intro Text over the top of the blurry/black scene
  // This guarantees it renders clearly on top of the black mask and never gets hidden!
  drawIntroText();

  // Draw Startup Welcome & Instruction Page overlay (on top of everything)
  drawWelcomePage();

  glutSwapBuffers();
}

void applySkySettings() {
  float rSky = 0.6f, gSky = 0.8f, bSky = 0.92f;
  float rAmb = 0.4f, gAmb = 0.4f, bAmb = 0.4f;
  float rDif = 0.9f, gDif = 0.9f, bDif = 0.9f;
  float lx = -15.0f, ly = 25.0f, lz = 25.0f;

  if (currentMode == MODE_MORNING_CINEMATIC) {
    float sunPhase = morningProgress / 0.25f; // 0.0 to 1.0
    if (sunPhase > 1.0f) sunPhase = 1.0f;
    rSky = 0.05f + (0.55f * sunPhase);
    gSky = 0.07f + (0.73f * sunPhase);
    bSky = 0.15f + (0.77f * sunPhase);

    rAmb = 0.08f + (0.32f * sunPhase);
    gAmb = 0.08f + (0.32f * sunPhase);
    bAmb = 0.15f + (0.25f * sunPhase);

    rDif = 0.15f + (0.75f * sunPhase);
    gDif = 0.15f + (0.75f * sunPhase);
    bDif = 0.25f + (0.65f * sunPhase);
    
    lx = 0.0f - (15.0f * sunPhase);
    lz = 10.0f + (15.0f * sunPhase);
  } else if (isNight) {
    rSky = 0.05f; gSky = 0.07f; bSky = 0.15f;
    rAmb = 0.08f; gAmb = 0.08f; bAmb = 0.15f;
    rDif = 0.15f; gDif = 0.15f; bDif = 0.25f;
    lx = 0.0f; lz = 10.0f;
  }

  glClearColor(rSky, gSky, bSky, 1.0f);
  float ambient[] = {rAmb, gAmb, bAmb, 1.0f};
  float diffuse[] = {rDif, gDif, bDif, 1.0f};
  float lightPos[] = {lx, ly, lz, 0.0f};

  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}

void init() {
  qobj = gluNewQuadric();
  gluQuadricNormals(qobj, GLU_SMOOTH);

  glClearColor(0.6f, 0.8f, 0.92f,
               1.0f); // Will be overridden by applySkySettings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_SMOOTH);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  // Lighting setup
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL); // Allow glColor to set material colors

  float lightPos[] = {-15.0f, 25.0f, 25.0f,
                      0.0f}; // Moved to side/front for deeper shadows
  float ambient[] = {0.4f, 0.4f, 0.4f, 1.0f};
  float diffuse[] = {0.9f, 0.9f, 0.9f, 1.0f};
  float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

  // Material properties for shininess
  float specMat[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glMaterialfv(GL_FRONT, GL_SPECULAR, specMat);
  glMateriali(GL_FRONT, GL_SHININESS, 64);

  // Smoothing/Anti-aliasing
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  applySkySettings(); // Apply initial day lighting
}

void reshape(int w, int h) {
  if (h == 0)
    h = 1;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (float)w / h, 0.1, 1000.0);
  glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
  // Dismiss the Welcome Page on ANY key press
  if (showWelcomePage) {
    showWelcomePage = false;
    // Let the key still trigger its action below (e.g. 'M' starts cinematic)
  }

  if (key == 'w' || key == 'W')
    camHeight += 0.2f;
  if (key == 's' || key == 'S')
    camHeight -= 0.2f;

  // Day / Night toggle
  if (key == 'n' || key == 'N') {
    isNight = !isNight;
    applySkySettings();
  }

  // Go inside the classroom instantly
  if (key == 'g' || key == 'G') {
    if (currentMode == MODE_EXTERIOR) {
      currentMode = MODE_CLASSROOM;
      posX = -1.5f; // Positioned near the interior door
      posZ = -0.5f;
      camHeight = 0.8f; // Match the new student eye level
      autoAngle = 0.0f;
      zoom = 0.0f;
    }
  }

  // Start Cinematic Morning Routine (Sunrise -> Lecture)
  if (key == 'm' || key == 'M') {
    if (currentMode == MODE_EXTERIOR) {
      currentMode = MODE_MORNING_CINEMATIC;
      morningProgress = 0.0f;
      isNight = true; // force start at night
      isStudentSeated = false;
      isTeacherTeaching = false;
      actStudentX = 0.45f; actStudentZ = 15.0f;
      actTeacherX = -5.0f; actTeacherZ = 15.0f; // Hidden initially
      targetDoorAngle = 0.0f;
      introTextAlpha = 0.0f;
      focusBlurAmount = 1.0f; // Start fully blurred for the waking up effect!
      birdProgress = 0.0f;
    }
  }

  // Campus Overview — return to exterior wide view
  if (key == 'u' || key == 'U') {
    currentMode = MODE_EXTERIOR;
    transitionProgress = 0.0f;
    targetDoorAngle = 0.0f;
    zoom = 15.0f;     // Default exterior zoom
    camHeight = 1.2f; // Default exterior height
    autoAngle = 0.0f; // Reset rotation
    isNight = false;
    applySkySettings();
  }

  // Return to Normal Scene (INSTANT POP OUT)
  if (key == 'b' || key == 'B') {
    currentMode = MODE_EXTERIOR;
    transitionProgress = 0.0f;
    targetDoorAngle = 0.0f;
    zoom = 15.0f;     // Default exterior zoom
    camHeight = 1.2f; // Default exterior height
    autoAngle = 0.0f; // Reset rotation
  }

  // Limits
  if (camHeight < -1.0f)
    camHeight = -1.0f;
  if (camHeight > 5.0f)
    camHeight = 5.0f;

  glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
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
    if (posX < -2.8f)
      posX = -2.8f;
    if (posX > 2.8f)
      posX = 2.8f;
    if (posZ < -6.3f)
      posZ = -6.3f;
    if (posZ > 0.45f)
      posZ = 0.45f;
  } else {
    if (key == GLUT_KEY_UP)
      zoom -= 0.5f; // Zoom In
    if (key == GLUT_KEY_DOWN)
      zoom += 0.5f; // Zoom Out
  }

  if (key == GLUT_KEY_RIGHT)
    autoAngle += 5.0f;
  if (key == GLUT_KEY_LEFT)
    autoAngle -= 5.0f;

  if (currentMode == MODE_EXTERIOR) {
    if (zoom < 4.0f)
      zoom = 4.0f;
    if (zoom > 30.0f)
      zoom = 30.0f;
  }

  glutPostRedisplay();
}

void idle() {
  // Drift clouds slowly across the sky
  cloudOffset += 0.01f;
  if (cloudOffset > 100.0f)
    cloudOffset = 0.0f;

  // Move students (reduced speed)
  studentWalkAngle += 0.2f;
  if (studentWalkAngle > 360.0f)
    studentWalkAngle -= 360.0f;

  // Move car
  carProgress += 0.1f;
  if (carProgress > 45.0f)
    carProgress = -45.0f;

  if (currentMode == MODE_TRANSITION) {
    // Stop spinning and align building to face camera
    autoAngle = autoAngle * 0.95f;
    if (fabs(autoAngle) < 0.1f)
      autoAngle = 0.0f;

    transitionProgress += 0.012f; // faster cinematic speed
    if (transitionProgress >= 1.0f) {
      transitionProgress = 1.0f;
      currentMode = MODE_CLASSROOM;
      // Hand over control to user inside the room
      posX = -1.5f; // Positioned near the new interior door
      posZ = -0.5f;
      camHeight = 0.8f; // Match the new student eye level
      autoAngle = 0.0f;
    }

    // Open door logic (door is at X=0.45, opens starting at progress 0.2, fully
    // open at 0.5)
    if (transitionProgress > 0.2f) {
      float doorP = (transitionProgress - 0.2f) / 0.3f;
      if (doorP > 1.0f)
        doorP = 1.0f;
      // Smooth ease
      float dEase = doorP * doorP * (3.0f - 2.0f * doorP);
      targetDoorAngle = dEase * -90.0f; // opens inwards
    } else {
      targetDoorAngle = 0.0f;
    }

    // Entering student logic (starts moving at progress 0.4, reaches inside
    // by 1.0)
    if (transitionProgress > 0.4f) {
      float walkP = (transitionProgress - 0.4f) / 0.6f;
      // Student starts at Z=1.35, moves into the building Z=-2.5
      enteringStudentZ = 1.35f - (walkP * 3.85f);
      enteringStudentX = 0.45f;
    } else {
      enteringStudentZ = 1.35f;
      enteringStudentX = 0.45f;
    }
  } else if (currentMode == MODE_TRANSITION_BACK) {
    transitionProgress -= 0.012f; // faster reverse speed
    if (transitionProgress <= 0.0f) {
      transitionProgress = 0.0f;
      currentMode = MODE_EXTERIOR;
    }

    // Door logic in reverse
    if (transitionProgress > 0.2f) {
      float doorP = (transitionProgress - 0.2f) / 0.3f;
      if (doorP > 1.0f)
        doorP = 1.0f;
      float dEase = doorP * doorP * (3.0f - 2.0f * doorP);
      targetDoorAngle = dEase * -90.0f;
    } else {
      targetDoorAngle = 0.0f;
    }

    // Student logic in reverse
    if (transitionProgress > 0.4f) {
      float walkP = (transitionProgress - 0.4f) / 0.6f;
      enteringStudentZ = 1.35f - (walkP * 3.85f);
      enteringStudentX = 0.45f;
    } else {
      enteringStudentZ = 1.35f;
      enteringStudentX = 0.45f;
    }
  } else if (currentMode == MODE_MORNING_CINEMATIC) {
    morningProgress += 0.0005f; // Slower, more elegant cinematic sequence (about 33 seconds total)
    if (morningProgress >= 1.0f) {
      morningProgress = 1.0f;
      currentMode = MODE_CLASSROOM;
      posX = -0.5f; posZ = 0.0f; camHeight = 1.2f; autoAngle = 0.0f;
      isTeacherTeaching = true;
      isNight = false; // FIX: Ensure we stay daytime!
      introTextAlpha = 0.0f;
      focusBlurAmount = 0.0f;
      birdProgress = 0.0f;
    }
    applySkySettings(); // Dynamic sunrise

    // --- Cinematic Intro Timeline (Waking up, blur clearing, bird flight) ---
    // 1. Text alpha: Perfect smooth sine envelope peaking at morningProgress = 0.08, fully gone by 0.16
    if (morningProgress < 0.16f) {
      float t = morningProgress / 0.16f;
      float s = sinf(t * 3.14159265f);
      introTextAlpha = s * s; // Smooth bell curve S-shape
    } else {
      introTextAlpha = 0.0f;
    }

    // 2. Focus Blur (Eyelids): Smooth S-Curve (Smoothstep) opening completely by 0.30
    if (morningProgress < 0.30f) {
      float t = morningProgress / 0.30f;
      focusBlurAmount = 1.0f - (t * t * (3.0f - 2.0f * t));
    } else {
      focusBlurAmount = 0.0f;
    }

    // 3. Birds flight: Starts flying at progress 0.12, proceeding continuously across sky
    if (morningProgress >= 0.12f) {
      float t = (morningProgress - 0.12f) / (1.0f - 0.12f);
      birdProgress = t * 2.5f; // fly smoothly across the horizon
    } else {
      birdProgress = 0.0f;
    }

    if (morningProgress < 0.25f) {
      // Phase 1: Sunrise - actors static
    } else if (morningProgress < 0.50f) {
       // Phase 2: Student walks up the path to the door
       float p = (morningProgress - 0.25f) / 0.25f;
       float w0_x = 18.0f, w0_z = 13.5f; // Start
       float w1_x = 9.25f, w1_z = 13.5f; // Sidewalk corner
       float w2_x = 9.25f, w2_z = 3.5f;  // Cobblestone top
       float w3_x = 5.00f, w3_z = 3.5f;  // Ramp base
       float w4_x = 5.00f, w4_z = 1.8f;  // Ramp top (plinth level)
       float w5_x = 4.05f, w5_z = 1.8f;  // Porch door corner
       float w6_x = 4.05f, w6_z = 0.7f;  // Door entrance
       if (p < 0.25f) {
           float t = p / 0.25f;
           actStudentX = w0_x + t * (w1_x - w0_x);
           actStudentZ = w0_z;
           actStudentAngle = -90.0f;
       } else if (p < 0.55f) {
           float t = (p - 0.25f) / 0.30f;
           actStudentX = w1_x;
           actStudentZ = w1_z + t * (w2_z - w1_z);
           if (t < 0.15f) {
               float rt = t / 0.15f;
               actStudentAngle = -90.0f * (1.0f - rt) + rt * (-180.0f);
           } else {
               actStudentAngle = -180.0f;
           }
       } else if (p < 0.70f) {
           float t = (p - 0.55f) / 0.15f;
           actStudentX = w2_x + t * (w3_x - w2_x);
           actStudentZ = w2_z;
           if (t < 0.25f) {
               float rt = t / 0.25f;
               actStudentAngle = -180.0f * (1.0f - rt) + rt * (-90.0f);
           } else {
               actStudentAngle = -90.0f;
           }
       } else if (p < 0.85f) {
           float t = (p - 0.70f) / 0.15f;
           actStudentX = w3_x;
           actStudentZ = w3_z + t * (w4_z - w3_z);
           if (t < 0.25f) {
               float rt = t / 0.25f;
               actStudentAngle = -90.0f * (1.0f - rt) + rt * (-180.0f);
           } else {
               actStudentAngle = -180.0f;
           }
       } else {
           float t = (p - 0.85f) / 0.15f;
           if (t < 0.60f) {
               float t2 = t / 0.60f;
               actStudentX = w4_x + t2 * (w5_x - w4_x);
               actStudentZ = w4_z;
               actStudentAngle = -90.0f;
           } else {
               float t2 = (t - 0.60f) / 0.40f;
               actStudentX = w5_x;
               actStudentZ = w5_z + t2 * (w6_z - w5_z);
               actStudentAngle = -180.0f;
           }
       }
       // Classroom Phase (morningProgress >= 0.50f)
       // Door remains open as students and teacher enter, and closes smoothly at 0.95f
       if (morningProgress < 0.95f) {
           targetDoorAngle = -90.0f;
       } else if (morningProgress < 0.98f) {
           float dp = (morningProgress - 0.95f) / 0.03f;
           targetDoorAngle = -90.0f * (1.0f - dp); // Smoothly close door
       } else {
           targetDoorAngle = 0.0f;
       }

       // Set isTeacherTeaching when teacher is fully seated/at podium (morningProgress >= 0.97f)
       if (morningProgress >= 0.97f) {
           isTeacherTeaching = true;
       } else {
           isTeacherTeaching = false;
       }
    }
  }

  glutPostRedisplay();
}

int main(int argc, char **argv) {
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
