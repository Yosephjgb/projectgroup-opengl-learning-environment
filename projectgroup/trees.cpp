// PROCEDURAL TREE GENERATION ALGORITHMS
// These functions use purely custom mathematical geometry for high-fidelity 3D tree generation.

// Custom geometric helper for branches to increase detail and avoid GLU overhead
void drawCustomBranch(float radiusBase, float radiusTop, float length, int slices, int stacks) {
    float angleStep = 6.2831853f / slices;
    float heightStep = length / stacks;
    for (int j = 0; j < stacks; j++) {
        float h1 = j * heightStep;
        float h2 = (j + 1) * heightStep;
        float r1 = radiusBase - (radiusBase - radiusTop) * ((float)j / stacks);
        float r2 = radiusBase - (radiusBase - radiusTop) * ((float)(j + 1) / stacks);
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
        
        float sx = sinf(ax) * size; float cx = cosf(ax) * size;
        float sy = sinf(ay) * size; float cy = cosf(ay) * size;
        float sz = sinf(az) * size; float cz = cosf(az) * size;
        
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
void drawOakBranch(float length, float radius, int depth, int maxDepth, int seed) {
    if (depth > maxDepth) return;
    
    glPushMatrix();
    glColor3f(0.25f - depth*0.02f, 0.20f - depth*0.015f, 0.15f);
    
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
            drawOakBranch(length * 0.75f, radius * 0.65f, depth + 1, maxDepth, subSeed);
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
    for (int i=0; i<4; i++) {
        glPushMatrix();
        glRotatef(i*90.0f + (seed%45), 0, 1, 0);
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
        glRotatef(sway + (seed%5), 1, 0, 0);
        drawCustomBranch(0.02f, 0.01f, segLen, 4, 1);
        glTranslatef(0.0f, segLen, 0.0f);
        // Leaf on vine
        if (i % 2 == 0) drawCustomLeafCluster(0.15f, 2, 0.45f, 0.65f, 0.25f);
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
            drawWillowVine(2.5f + (seed%10)*0.1f, 8, seed * v);
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
            glRotatef(80.0f + (seed%10), 1, 0, 0);
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
void drawJacarandaBranch(float length, float radius, int depth, int maxDepth, int seed) {
    if (depth > maxDepth) return;
    
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
            glTranslatef(0, -length*2.0f, 0);
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
            drawJacarandaBranch(length * 0.8f, radius * 0.6f, depth + 1, maxDepth, subSeed);
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
void drawAcaciaBranch(float length, float radius, int depth, int maxDepth, int seed) {
    if (depth > maxDepth) return;
    glPushMatrix();
    glColor3f(0.5f, 0.45f, 0.4f);
    
    // Acacia branches bend sharply horizontally
    if (depth > 0) {
        glRotatef(50.0f + (seed%20), 1, 0, 0);
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
            drawAcaciaBranch(length * 0.9f, radius * 0.6f, depth + 1, maxDepth, subSeed);
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
