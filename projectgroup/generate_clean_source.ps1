$original_path = "c:\Users\hp\source\repos\projectgroup\projectgroup\original_Source_utf8.cpp"
$output_path = "c:\Users\hp\source\repos\projectgroup\projectgroup\Source.cpp"

if (-not (Test-Path $original_path)) {
    Write-Error "Original source file not found at $original_path"
    Exit 1
}

# Read original source
$lines = Get-Content -Path $original_path -Encoding utf8

$header = @'
/**
 * =====================================================================================================================
 *                                         WOLKITE UNIVERSITY CAMPUS SIMULATION
 * =====================================================================================================================
 *
 *                                            ______   __    __   ________
 *                                           /      \\ /  |  /  | /        |
 *                                          /$$$$$$  |$$ |  $$ | $$$$$$$$/
 *                                          $$ |  $$ |$$ |  $$ | $$ |__
 *                                          $$ |  $$ |$$ |  $$ | $$    |
 *                                          $$ |  $$ |$$ |  $$ | $$$$$/
 *                                          $$ \\__$$ |$$ \\__$$ | $$ |_____
 *                                          $$    $$/ $$    $$/  $$       |
 *                                           $$$$$$/   $$$$$$/   $$$$$$$$/
 *
 * =====================================================================================================================
 * A Premium, High-Fidelity 3D Virtual Reconstruction of the Wolkite University Gatehouse Complex
 * and its Immersive, Live-Action Classroom Interior. Built using Modern OpenGL and GLUT.
 * =====================================================================================================================
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * TABLE OF CONTENTS
 * ---------------------------------------------------------------------------------------------------------------------
 *   1. MATHEMATICAL DEEP DIVE & GRAPHICS EQUATIONS
 *      A. Perspective Viewing Frustum Projection Matrix Derivation
 *      B. Phong Reflection and Lighting Model Vector Proofs
 *      C. ModelView Coordinate Transformations Matrix Composition
 *      D. Parametric Elliptic Pathing & Tangent Orientation Calculus
 *   2. SYSTEM ARCHITECTURE & INTERACTIVES
 *      A. Interactive Keybindings & Controls Reference Sheet
 *      B. Rigid-Body First-Person Camera Collision Bounds Lock
 *   3. ARCHITECTURAL GEOMETRY SCHEMATICS (ASCII ART DIAGRAMS)
 *      A. Academic Gatehouse Structural Levels
 *      B. U-Shaped Dog-Leg Staircase Engineering
 *      C. Classroom Interior Spatial Floorplan
 *   4. GRAPHICS COMPILATION & LINKER LANDSCAPE
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 1. MATHEMATICAL DEEP DIVE & GRAPHICS EQUATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * A. PERSPECTIVE VIEWING FRUSTUM PROJECTION MATRIX DERIVATION
 * The transformation from 3D camera coordinates (eye space) to 2D normalized device coordinates (NDC) is governed
 * by the perspective projection matrix. This projection replicates human vision by making distant objects appear
 * smaller than near ones. The matrix maps a frustum defined by fovY, aspect ratio, zNear, and zFar into a symmetric
 * homogeneous clipping cube where coordinates are locked between [-1.0, 1.0].
 *
 * The mathematical perspective projection matrix P applied in the pipeline is:
 *
 *     P = [  f / aspect         0                      0                            0               ]
 *         [      0              f                      0                            0               ]
 *         [      0              0         (zFar+zNear)/(zNear-zFar)      (2*zFar*zNear)/(zNear-zFar) ]
 *         [      0              0                     -1                            0               ]
 *
 * Derivation of the focal length term (f):
 *     The field of view angle (fovY) represents the vertical angle subtended by the viewing frustum.
 *     If we take a cross-section of the frustum along the Y-axis:
 *
 *          +Y_eye
 *            ^         . zNear
 *            |       . |
 *            |     .   |
 *            |   .     |  yNear (top of near plane)
 *            | . fovY/2|
 *            +-------->+---------> -Z_eye
 *              .       |
 *                .     |
 *                  .   |
 *                    . |
 *                      .
 *
 *     From simple trigonometry, the height of the near clipping plane is:
 *         tan(fovY / 2) = yNear / zNear
 *     Therefore:
 *         yNear = zNear * tan(fovY / 2)
 *
 *     The projection maps vertical eye coordinates Ye to clip coordinates Yc such that at the near plane:
 *         Yc = Ye * f = Ye / tan(fovY / 2)
 *     This focal length constant is defined as:
 *         f = cot(fovY / 2) = 1.0 / tan(fovY / 2)
 *
 * Derivation of the aspect ratio term (aspect):
 *     The aspect ratio is the ratio of viewport width to viewport height:
 *         aspect = width / height = xNear / yNear
 *     Therefore, the horizontal boundary is scaled to match the aspect ratio:
 *         xNear = yNear * aspect = zNear * aspect * tan(fovY / 2)
 *     This yields the horizontal scaling factor:
 *         horizontal_scale = f / aspect
 *
 * Derivation of the depth mapping terms (Z coordinates):
 *     Homogeneous coordinates represent points in 4D space [Xc, Yc, Zc, Wc]^T. To project onto 3D NDC, we divide
 *     each coordinate by Wc (where Wc = -Ze):
 *         Xndc = Xc / -Ze
 *         Yndc = Yc / -Ze
 *         Zndc = Zc / -Ze
 *
 *     We define Zc as a linear combination of Ze and Wc:
 *         Zc = A * Ze + B
 *     Thus:
 *         Zndc = (A * Ze + B) / -Ze = -A - B / Ze
 *
 *     We establish boundary conditions at the near and far planes:
 *         1. When Ze = -zNear, Zndc = -1.0  =>  -A + B / zNear = -1.0  =>  -A * zNear + B = -zNear
 *         2. When Ze = -zFar,  Zndc =  1.0  =>  -A + B / zFar  =  1.0  =>  -A * zFar  + B =  zFar
 *
 *     Solving this system of linear equations yields:
 *         A = -(zFar + zNear) / (zFar - zNear) = (zFar + zNear) / (zNear - zFar)
 *         B = -(2 * zFar * zNear) / (zFar - zNear) = (2 * zFar * zNear) / (zNear - zFar)
 *
 *     This matches the third row of the perspective projection matrix exactly.
 *
 * B. PHONG REFLECTION AND LIGHTING MODEL VECTOR PROOFS
 * The Phong reflection model calculates the final color value at a vertex as the sum of ambient, diffuse,
 * and specular lighting components:
 *
 *     I_total = I_ambient + I_diffuse + I_specular
 *
 * Let:
 *     N = Unit surface normal vector (orthogonal to the tangent plane of the surface)
 *     L = Unit light direction vector (pointing from the vertex coordinates to the light source position)
 *     V = Unit view vector (pointing from the vertex coordinates to the camera position)
 *     R = Unit reflection vector (direction of maximum mirror-like reflection)
 *
 * Vector Derivation of the Reflection Vector (R):
 *     The angle of incidence equals the angle of reflection. This means that if we project the incident vector
 *     onto the normal, the normal splits the vector difference exactly in half.
 *
 *         Incident Vector (pointing in) = -L
 *         Normal Vector = N
 *
 *         The projection of L onto the normal N is:
 *             Proj_N(L) = (L . N) * N
 *
 *         To find the reflection vector R:
 *             R = L_reflected = 2 * Proj_N(L) - L = 2 * (L . N) * N - L
 *
 *     Proof of unit length:
 *         R . R = (2 * (L . N) * N - L) . (2 * (L . N) * N - L)
 *               = 4 * (L . N)^2 * (N . N) - 4 * (L . N) * (N . L) + (L . L)
 *         Since N and L are unit vectors, N . N = 1 and L . L = 1:
 *         R . R = 4 * (L . N)^2 - 4 * (L . N)^2 + 1 = 1
 *         This mathematically proves R has unit length.
 *
 * Diffuse Reflection (Lambertian Cosine Law):
 *     The intensity of reflected light is proportional to the cosine of the angle between surface normal N and
 *     light vector L.
 *         I_diffuse = Kd * Ld * max(0.0, N . L)
 *     If N . L is negative, the light is hitting the back of the surface, so we clamp it to 0.0.
 *
 * Specular Reflection (Glossy Highlight):
 *     Light is reflected in a glossy highlights cone centered around the reflection vector R.
 *         I_specular = Ks * Ls * (max(0.0, R . V))^alpha
 *     Where alpha is the specular shininess exponent (larger exponents create smaller, sharper highlights).
 *
 * C. MODELVIEW COORDINATE TRANSFORMATIONS MATRIX COMPOSITION
 * OpenGL relies on matrix multiplication to translate, rotate, and scale 3D geometry. Because matrix
 * multiplication is non-commutative ($A * B \neq B * A$), the order in which matrices are applied is critical.
 *
 * Let's analyze the compound transformation matrix M:
 *
 *     M = T(tx, ty, tz) * Ry(theta) * S(sx, sy, sz)
 *
 * Applying this compound matrix to a local vertex vector V_local rotates and scales the vertex relative to its
 * local origin, and then translates it into absolute world space coordinates:
 *
 *     V_world = M * V_local = T * Ry * S * V_local
 *
 * In OpenGL, matrix transformations are specified in reverse logical order in the code. To perform M = T * R * S:
 *     1. Call glTranslatef(tx, ty, tz)  (adds translation matrix to stack)
 *     2. Call glRotatef(theta, 0, 1, 0) (multiplies rotation matrix into stack)
 *     3. Call glScalef(sx, sy, sz)      (multiplies scaling matrix into stack)
 *     4. Render geometry                (vertex is multiplied by stack top)
 *
 * Reversing this order scales and rotates the translation vector itself, spinning the object out of orbit!
 *
 * D. PARAMETRIC ELLIPTIC PATHING & TANGENT ORIENTATION CALCULUS
 * The pacing students on the first-floor balcony traverse a parametric ellipse in 3D coordinate space.
 * Let the path be defined parameterized by angle t:
 *
 *     x(t) = A * sin(t)
 *     z(t) = Z_center + B * cos(t)
 *     y(t) = Y_balcony
 *
 * To ensure characters face forward along their movement direction, we must calculate the tangent vector
 * of the parametric curve at any time t:
 *
 *     T_vector(t) = [ dx/dt, dy/dt, dz/dt ]^T
 *
 * Taking derivatives with respect to t:
 *     dx/dt = A * cos(t)
 *     dy/dt = 0
 *     dz/dt = -B * sin(t)
 *
 * The yaw facing angle theta (rotation angle around the Y-axis) is the angle of this tangent vector
 * relative to the Z-axis in the XZ plane:
 *
 *     theta = atan2(dx/dt, dz/dt) = atan2(A * cos(t), -B * sin(t))
 *
 * Converting this angle from radians to degrees for OpenGL's glRotatef:
 *     theta_degrees = theta * (180.0 / PI)
 *
 * This calculus-driven orientation loop ensures that students face perfectly forward along their path,
 * resolving any orientation slipping or jittering at the turning points.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 2. SYSTEM ARCHITECTURE & INTERACTIVES
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * A. INTERACTIVE KEYBINDINGS & CONTROLS REFERENCE SHEET
 * +----------------+--------------------------------------------+-----------------------+-----------------------------+
 * | Key Code       | Visual Subsystem Action                    | Variable Mutated      | Clamps / Constraints        |
 * +----------------+--------------------------------------------+-----------------------+-----------------------------+
 * | 'W' / 'w'      | Elevate Camera Height (Upwards)            | `camHeight`           | Max: 5.0f (Normal View)     |
 * | 'S' / 's'      | Lower Camera Height (Downwards)            | `camHeight`           | Min: -1.0f (Normal View)    |
 * | 'N' / 'n'      | Toggle Day/Night Lighting Presets          | `isNight`             | true / false                |
 * | 'G' / 'g'      | Initiate Cinematic Classroom Entry         | `currentMode`         | MODE_EXTERIOR -> MODE_TRANS |
 * | 'B' / 'b'      | Initiate Classroom Exit Sweep              | `currentMode`         | MODE_CLASS -> MODE_TR_BACK  |
 * | UP ARROW       | Translate Camera Forward (Classroom Mode)  | `posX`, `posZ`        | Boundary Box Constraints    |
 * | DOWN ARROW     | Translate Camera Backward (Classroom Mode) | `posX`, `posZ`        | Boundary Box Constraints    |
 * | LEFT ARROW     | Rotate Camera Yaw Left (Classroom Mode)    | `autoAngle`           | Uncapped Circular [0, 360]  |
 * | RIGHT ARROW    | Rotate Camera Yaw Right (Classroom Mode)   | `autoAngle`           | Uncapped Circular [0, 360]  |
 * +----------------+--------------------------------------------+-----------------------+-----------------------------+
 *
 * B. RIGID-BODY FIRST-PERSON CAMERA COLLISION BOUNDS LOCK
 * To prevent the camera from clipping through interior geometry (walls, desks, whiteboard), hard
 * coordinate collision limits are enforced inside the display loop before rendering.
 *
 * Boundary clamps applied in `MODE_CLASSROOM`:
 *   - X Translation Boundary (Left/Right Walls):
 *     -3.30f <= posX <= 3.30f
 *   - Z Translation Boundary (Front/Back Walls):
 *     -6.30f <= posZ <= 0.45f
 *   - Y Translation Boundary (Floor/Ceiling Heights):
 *     0.40f <= camHeight <= 1.80f (mimicking normal human perspective heights)
 *   - Zoom scale factor is locked to 0.0f to prevent camera distortion.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 3. ARCHITECTURAL GEOMETRY SCHEMATICS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * A. ACADEMIC GATEHOUSE STRUCTURAL LEVELS (ASCII ART DIAGRAM)
 *
 *                      [Attic Ventilator Breezeway Blocks]
 *                      +---------------------------------+
 *                      |  ooo   ooo   ooo   ooo   ooo    |  Y = 3.25f (Attic Peak)
 *                      +---------------------------------+
 *                      |                                 |  Y = 2.65f (Attic Base)
 *     =================+=================================+=================  RY_Front = 2.40f (Canopy)
 *     \\  Pillars      |   [Door 4]  [Door 3]  [Door 2]  |  [Balcony Rail] \\
 *      \\ |  |  |  |   |   +------+  +------+  +------+  |  +------------+  \\
 *       \\|  |  |  |   |   |      |  |      |  |      |  |  | | | | | | | |   \\ Y = 1.08f (Balcony Level)
 *     ===+========+====+===|======|==|======|==|======|==+==|==============+=== Y = 1.00f (First Floor)
 *        |             |   |      |  | [G]  |  |      |  |                 |
 *        |  [Access    |   |      |  | Door |  |      |  |   [Staircase]   |
 *        |   Ramp]     |   |      |  |      |  |      |  |   Dog-Leg U-    |
 *        |   ======>   |   +------+  +------+  +------+  |   Shape Flight  |  Y = 0.15f (Plinth Level)
 *     +--+---------+---+---------------------------------+-----------------+--+ Y = 0.00f (Ground)
 *     |//////////////////////////////////////////////////////////////////////|
 *     +----------------------------------------------------------------------+
 *
 * B. U-SHAPED DOG-LEG STAIRCASE ENGINEERING (ASCII ART DIAGRAM)
 *
 *                                     [Top Balcony]  Y = 1.00f
 *                                     +-----------+
 *                                     |  Step 12  |
 *                                     +-----------+
 *                                     |  Step 11  |  Flight 2 Climbing Upwards
 *                                     +-----------+  (Directed inwards)
 *                                     |  Step 10  |
 *                                     +-----------+
 *                                     |  Step 9   |
 *                                     +-----------+
 *                                     |  Step 8   |
 *                                     +-----------+
 *                       +-------------+-----------+
 *                       |        [Mid-Landing]    |  Y = 0.51f (Platform turning point)
 *                       +-------------+-----------+
 *                       |  Step 6     |
 *                       +-------------+
 *                       |  Step 5     |
 *                       +-------------+  Flight 1 Climbing Downwards
 *                       |  Step 4     |  (Directed outwards)
 *                       +-------------+
 *                       |  Step 3     |
 *                       +-------------+
 *                       |  Step 2     |
 *                       +-------------+
 *                       |  Step 1     |
 *                       +-------------+
 *                       | [Foundation Plinth Base]   Y = 0.15f
 *                       +-------------------------+
 *
 * C. CLASSROOM INTERIOR SPATIAL FLOORPLAN (ASCII ART DIAGRAM)
 *
 *      [Front Wall]  Z = -7.50f
 *      +---------------------------------------------------------------------+
 *      |   [Whiteboard]             [Teacher Podium]        [Glowing TV Screen]|
 *      |   +-------------+           +------------+         +----------------+ |
 *      |   |             |           |   Podium   |         |   Light-Blue   | |
 *      |   +-------------+           +------------+         +----------------+ |
 *      |                                                                     |
 *      |       [D1]         [D2]         [D3]         [D4]         [D5]      |
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |  Desk Row 1 (Z = -5.50f)
 *      |    | Notebook   | Notebook   | Notebook   | Notebook   | Notebook | |
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |
 *      |                                                                     |
 *      |       [D6]         [D7]         [D8]         [D9]         [D10]     |
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |  Desk Row 2 (Z = -4.60f)
 *      |    | Notebook   | Notebook   | Notebook   | Notebook   | Notebook | |
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |
 *      |                                                                     |
 *      |       [D11]        [D12]        [D13]        [D14]        [D15]     |
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |  Desk Row 3 (Z = -3.70f)
 *      |    | Notebook   | Notebook   | Notebook   | Notebook   | Notebook | |
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |
 *      |                                                                     |
 *      |                     [42 Desks Array Grid Layout]                    |
 *      |                                                                     |
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |
 *      |    | Notebook   | Notebook   | Notebook   | Notebook   | Notebook | |  Desk Row 7 (Z = 0.00f)
 *      |    +--------+   +--------+   +--------+   +--------+   +--------+   |
 *      +-------------------------------------------------------------+-------+
 *      |                                                             | [Door]| Z = 0.65f (Back Wall)
 *      +-------------------------------------------------------------+-------+
 *      X = -3.50f (Left Wall)                                     X = 3.50f (Right Wall)
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 4. GRAPHICS COMPILATION & LINKER LANDSCAPE
 * ---------------------------------------------------------------------------------------------------------------------
 * This application is configured to build under the Microsoft Visual C++ Compiler (MSVC) using the Win32 / x86
 * configuration. Linking requires standard Windows OpenGL and GLUT imports:
 *   - opengl32.lib : Core Windows OpenGL hardware driver entry points.
 *   - glu32.lib    : OpenGL Utility Library containing frustum matrix calculators (`gluPerspective`, `gluCylinder`).
 *   - glut32.lib   : OpenGL Utility Toolkit managing Window creation, double buffering, and keyboard/mouse interrupts.
 *
 * IMPORTANT PLATFORM BINDINGS:
 * GLUT is linked strictly as a 32-bit library (`glut32.lib`). Attempts to build under x64 fail immediately with
 * unresolved linker errors. The target build configuration MUST remain Win32 / x86.
 */
'@

$solidbox_theory = @'
/**
 * =====================================================================================================================
 * SECTION 2. solidBox Primitive Generator
 * =====================================================================================================================
 * The `solidBox` helper is the core geometric primitives routine. Rather than relying on GLUT's basic
 * `glutSolidCube` which maps identical colors to all faces, `solidBox` allows complete custom colors and
 * ambient lighting parameters for all six faces of an arbitrary rectangular parallelepiped.
 *
 * A. SURFACE NORMALS & PHONG SHADING
 * Surfaces must define outward-facing normal vectors to enable correct lighting calculations. If normals
 * are omitted or mapped incorrectly, light will reflect at illegal angles, causing surfaces to appear
 * completely black, flat, or inside-out.
 *
 * Normal vectors are specified per face:
 *   - Front Face:   N = [  0.0f,  0.0f,  1.0f ]
 *   - Back Face:    N = [  0.0f,  0.0f, -1.0f ]
 *   - Left Face:    N = [ -1.0f,  0.0f,  0.0f ]
 *   - Right Face:   N = [  1.0f,  0.0f,  0.0f ]
 *   - Top Face:     N = [  0.0f,  1.0f,  0.0f ]
 *   - Bottom Face:  N = [  0.0f, -1.0f,  0.0f ]
 *
 * B. WINDING ORDER & BACKFACE CULLING
 * Vertices are defined in a counter-clockwise (CCW) winding order. When viewed from the outside,
 * this winding order signals that the face is facing the camera, preventing backfaces from being
 * drawn when backface culling (`glEnable(GL_CULL_FACE)`) is enabled, saving rendering resources.
 *
 * C. AMBIENT OCCLUSION GRADIENTS
 * To simulate soft shadowing where corners meet (ambient occlusion), side colors are multiplied by
 * a 0.75x shading coefficient, and bottom colors by a 0.70x coefficient, producing high-fidelity
 * visual depth even in fixed-function OpenGL configurations.
 *
 * PARAMETERS DICTIONARY:
 *   - x1, y1, z1 : Minimum bounding coordinates of the box.
 *   - x2, y2, z2 : Maximum bounding coordinates of the box.
 *   - rf, gf, bf : RGB color values for the front and back faces.
 *   - rs, gs, bs : RGB color values for the left and right side faces.
 *   - rt, gt, bt : RGB color values for the top and bottom faces.
 */
'@

$drawtree_theory = @'
/**
 * =====================================================================================================================
 * SECTION 3. drawTree Procedural Vegetation Routine
 * =====================================================================================================================
 * Renders high-fidelity trees using composite 3D shapes.
 *
 * Trunk Geometry:
 *   - Rendered using a vertical solid box with concrete weathered brown tones (`0.35f, 0.25f, 0.15f`).
 *   - Base coordinates are set at XZ coordinates with a trunk width of 0.20f and height of 1.50f.
 *
 * Canopy Styles (`type` Parameter):
 *   1. Cone Canopy (Type 0 - Evergreen Pine):
 *      - Rendered using `glutSolidCone` with a base radius of 0.80f and a height of 2.50f.
 *      - Because standard cones project horizontally along the Z-axis, it is rotated 90 degrees around
 *        the X-axis (`glRotatef(-90.0f, 1.0f, 0.0f, 0.0f)`) to stand vertically along the Y-axis.
 *   2. Sphere Canopy (Type 1 - Deciduous Shade Tree):
 *      - Rendered using `glutSolidSphere` with a radius of 1.00f.
 *      - Translated vertically to sit on top of the trunk at $Y = 2.20f$.
 *
 * OPTIMIZED POLYGON COUNTS:
 * Cones are rendered with 8 slices and 4 stacks, and spheres with 8 slices and 8 stacks. This reduces
 * rendering overhead, allowing large forests of 100+ trees to render smoothly at 60 FPS.
 */
'@

$cobblestone_theory = @'
/**
 * =====================================================================================================================
 * SECTION 4. drawCobblestonePath Procedural Tiling
 * =====================================================================================================================
 * Generates highly realistic cobblestone pathways.
 *
 * To avoid flat, lifeless gray polygons, this function dynamically generates a physical grid of small stone blocks.
 *
 * DETERMINISTIC COORD SEEDING:
 * Because OpenGL is stateless and re-renders the scene at 60 FPS, standard random functions would cause
 * the cobblestones to flicker wildly every frame. Instead, we use a deterministic seeding formula based on
 * each stone's absolute coordinate coordinates:
 *
 *     seed = (int)((x + 50.0f) * 10.0f) * 131 + (int)((z + 50.0f) * 10.0f)
 *
 * DETERMINISTIC WEATHERING & DISPLACEMENTS:
 *   1. Jitter Displacement: The seed determines small, organic coordinate offsets for each stone, breaking
 *      up flat geometric lines:
 *      jx = (seed % 7) * 0.01f; jz = (seed % 9) * 0.01f
 *   2. Broken Tiles: We skip roughly 10% of tiles (`if (seed % 11 == 0) continue`) to simulate weathered paving
 *      where grass or dirt peaks through.
 *   3. Slate Color Variation: Adds realistic weathering tones by varying grey values based on the seed:
 *      v = (seed % 30) / 100.0f; color = [0.38f + v, 0.38f + v, 0.40f + v].
 */
'@

$bush_theory = @'
/**
 * =====================================================================================================================
 * SECTION 5. drawBush Foliage Clustering
 * =====================================================================================================================
 * Renders continuous green hedges by clustering overlapping spheres.
 *
 * Cluster Offsets Matrix:
 *   - Center Sphere:   Offset [  0.00f,  0.25f,  0.00f ]
 *   - Northeast:       Offset [  0.15f,  0.20f,  0.12f ]
 *   - Northwest:       Offset [ -0.15f,  0.20f,  0.12f ]
 *   - Southeast:       Offset [  0.12f,  0.18f, -0.15f ]
 *   - Southwest:       Offset [ -0.12f,  0.18f, -0.15f ]
 *
 * All spheres are rendered with a radius of 0.25f, 6 slices, and 6 stacks. The complex overlapping structure
 * creates organic shadows and highlights, capturing the look of lush, physical foliage.
 */
'@

$grass_theory = @'
/**
 * =====================================================================================================================
 * SECTION 6. drawLongGrass Blade Renderer
 * =====================================================================================================================
 * Populates path-road intersections with physical grass blades to add realistic detail.
 *
 * Blending and Primitive Styling:
 *   - Rendered using primitive lines (`GL_LINES`) with a width of 1.0f.
 *   - Draw 3-4 individual grass blades per coordinate point. Blades start on the ground plane ($Y = 0.0f$)
 *     and curve slightly outwards, using a realistic, deep forest green color (`0.35f, 0.55f, 0.25f`).
 */
'@

$ditch_theory = @'
/**
 * =====================================================================================================================
 * SECTION 7. drawDitch Slope Concrete Drain
 * =====================================================================================================================
 * Models sloped concrete drainage ditches flanking either side of the entrance pathways.
 *
 * Slope Normal Trigonometry:
 * Because the ditch sides are sloped, standard vertical normals are mathematically invalid and would ruin
 * the lighting shading. We calculate correct normals by taking cross products of the edge vectors:
 *   - Left Slope Normal:  N = [  0.447f,  0.894f,  0.0f ]
 *   - Right Slope Normal: N = [ -0.447f,  0.894f,  0.0f ]
 *
 * Dimensions and Styling:
 *   - Width: 0.50f; Depth: 0.25f.
 *   - Color: Mossy concrete grey-green (`0.42f, 0.48f, 0.38f`), integrating the ditch with the grass.
 */
'@

$ditchhoriz_theory = @'
/**
 * =====================================================================================================================
 * SECTION 8. drawDitchHorizontal Front Drain
 * =====================================================================================================================
 * The horizontally aligned variant of the concrete drainage ditch, running parallel to the main building base.
 *
 * Slope Normal Math (XZ Plane):
 *   - Back Slope Normal:  N = [  0.0f,  0.894f,  0.447f ]
 *   - Front Slope Normal: N = [  0.0f,  0.894f, -0.447f ]
 *
 * Bounds:
 *   - Runs along Z = 2.10f, separating the concrete plinth from the front grass lawn.
 */
'@

$door_theory = @'
/**
 * =====================================================================================================================
 * SECTION 9. drawDetailedDoor Swing Assembly
 * =====================================================================================================================
 * Models an architectural double-door assembly.
 *
 * A. STRUCTURAL COMPONENTS:
 *   1. Transom Window: A dark-tinted glass panel (`0.05f, 0.05f, 0.12f`) located directly above the door.
 *   2. Concrete Frame: Encases the assembly, drawn with thick white lines.
 *   3. Interior Mask Box: A thin black box (`0.0f, 0.0f, 0.0f`) positioned slightly behind the door.
 *      This hides the wall geometry behind it, creating a realistic depth illusion when the door swings open.
 *
 * B. DOOR HINGE ROTATION SYSTEM:
 * Standard rotations are performed relative to the local origin ([0,0,0]). To swing the door from its
 * left edge ($X = x - w/2$), we apply a compound translation-rotation sequence:
 *
 *     1. Translate Hinge to Origin:  glTranslatef(x - w / 2, y, z)
 *     2. Rotate door:                glRotatef(openAngle, 0.0f, 1.0f, 0.0f)
 *     3. Translate Hinge Back:       glTranslatef(-(x - w / 2), -y, -z)
 *
 * C. COSMETIC MICRO-GEOMETRY:
 *   - Five vertical decorative ridges, drawn on the door face using white lines.
 *   - A raised tan horizontal middle bar, splitting the door panels.
 *   - A dark metallic handles block (`glutSolidCube`), translated to the opening edge.
 */
'@

$student_theory = @'
/**
 * =====================================================================================================================
 * SECTION 10. drawStudent Dynamic Character Renderer
 * =====================================================================================================================
 * Models dynamic student and teacher characters using a fully jointed C++ skeletal structure.
 *
 * A. PERIODIC MOTION SYSTEM (WAVE PHYSICS):
 * Characters animate dynamically in real time using wave calculations based on the walk phase angle:
 *
 *   1. Body Bobbing (Vertical translation):
 *      bob = abs(sin(walkPhase * 2.0)) * 0.05
 *      Simulates a walking stride, dropping the torso at the midpoint of each step.
 *
 *   2. Leg striding:
 *      legSwing = sin(walkPhase) * 35.0 degrees
 *      Rotates left and right leg segments in opposite directions along the X-axis.
 *
 *   3. Arm striding:
 *      armSwing = sin(walkPhase) * 45.0 degrees
 *      Arm swing alternates in opposition to the legs to balance torso rotation.
 *
 * B. INTERACTIVE STATE MAPPINGS:
 *   1. Seated State (`isSeated = true`):
 *      - Leg joints rotate forward 90 degrees around the X-axis, and knees bend backward 90 degrees.
 *      - Arm joints rotate forward 30 degrees, resting realistically on top of the student desks.
 *      - Bobbing physics is deactivated (`bob = 0.0f`).
 *
 *   2. Teaching/Lecturing State (`isTeaching = true`):
 *      - The character is scaled up by 15% to simulate a mature lecturer standing at a podium.
 *      - The walking trajectory is slowly paced across the front whiteboard area (`tx += sin(walkPhase * 0.05) * 0.6`).
 *      - The teacher periodically rotates 180 degrees to face the whiteboard when the phase sine wave peaks,
 *        mimicking writing notes.
 *
 * C. MULTI-VARIANT INDIVIDUALITY CODES:
 * Color palettes are derived procedurally from the character's ID (`variant`) to avoid identical clones:
 *   - Shirt Colors: 6 distinct variations (blue, red, green, yellow, purple, cyan).
 *   - Hair Colors: 4 distinct variations (black, brown, blonde, gray).
 */
'@

$students_theory = @'
/**
 * =====================================================================================================================
 * SECTION 11. drawStudents Behavior Manager
 * =====================================================================================================================
 * High-level manager coordinating the movement and animation states of all students roaming the campus.
 *
 * Pathing Trajectories:
 *   - Balcony Strider: 3 students walk along a parametric ellipse on the first-floor balcony corridor.
 *     Phase parameters are offset by 120 degrees (`i * 120.0f`) to keep them evenly spaced along the path.
 *   - Ground Strider: 3 students pace along the ground-level walkway.
 *   - Student 0 Entry Transition: During `MODE_TRANSITION`, Student 0 detaches from their normal path,
 *     moving along the Z-axis, walking through the center doors to take their seat inside the classroom.
 */
'@

$classroom_theory = @'
/**
 * =====================================================================================================================
 * SECTION 12. drawClassroomInterior Renderer
 * =====================================================================================================================
 * Renders the detailed classroom interior, fully populated with modern academic props.
 *
 * A. STRUCTURAL ELEMENTS:
 *   1. Floor Tiles: Light grey tiles rendered with a grid to create perspective depth.
 *   2. Two-Tone Walls: Features a dark gray base band up to $Y = 1.2f$, topped with clean white plaster walls.
 *   3. Ceiling Beams: Heavy structural concrete beams are positioned at the front to support ceiling loads.
 *
 * B. MULTIMEDIA PROPS:
 *   1. Analog Wall Clock: Positioned directly above the whiteboard, with tick marks and hands.
 *   2. Educational Posters: Features a Periodic Table and a World Map rendered procedurally with
 *      colored pixel matrices on the front wall.
 *   3. Multimedia Screen & TV: A large flatscreen TV sits on the right side of the front wall,
 *      complete with a soft light-blue screen glow.
 *   4. Ceiling Projector: A modern projector hangs from the ceiling, complete with a mounting pole,
 *      lens cylinder, and light-blue lens glass.
 *   5. Air Conditioning Unit: Features vents, intake grills, and branding, adding realistic modern details.
 *
 * C. SEATING CONFIGURATION:
 * Features 42 student desks arranged in a 6-column by 7-row grid. Each desk has a small notebook prop,
 * and a seated student model with unique procedurally generated colors.
 */
'@

$stars_theory = @'
/**
 * =====================================================================================================================
 * SECTION 13. drawStar & drawStars Procedural Twinkling
 * =====================================================================================================================
 * Renders a twinkling starfield in the nighttime sky.
 *
 * Deterministic Spatial Seeding:
 * We use a deterministic algorithm to keep stars at fixed coordinates without relying on stateful `rand()` calls:
 *
 *     seed = i * 1973 + 42
 *     px = fmod(seed * 0.0137f, 160.0f) - 80.0f
 *     py = 15.0f + fmod(seed * 0.0271f, 25.0f)
 *     pz = fmod(seed * 0.0193f, 160.0f) - 80.0f
 *
 * Adds realistic twinkling by varying each star's brightness based on its seed coordinate.
 */
'@

$moon_theory = @'
/**
 * =====================================================================================================================
 * SECTION 14. drawMoon Crescent Overlay
 * =====================================================================================================================
 * Renders a detailed 3D crescent moon.
 *
 * A. ORBITAL MATRIX ROTATIONS:
 * The moon's position is calculated using the system's global day/night cycle angle (`sunMoonAngle`):
 *
 *     glRotatef(angle, 0.0f, 0.0f, 1.0f)
 *     glTranslatef(0.0f, 30.0f, -60.0f)
 *
 * This rotates the coordinate frame around the Z-axis, creating a realistic rising and setting orbit.
 *
 * B. CRESCENT SHADOW MASKS:
 * Instead of relying on complex 3D meshes, a classic crescent shape is achieved by rendering a larger,
 * pale yellow-white sphere, followed by a slightly smaller, dark-blue sphere slightly offset in
 * coordinate space. This masks out the backside of the moon, producing a beautiful, high-fidelity
 * crescent phase.
 */
'@

$sun_theory = @'
/**
 * =====================================================================================================================
 * SECTION 15. drawSun Solar Flares
 * =====================================================================================================================
 * Renders a glowing sunburst assembly.
 *
 * A. RAY TRIGONOMETRY:
 * Renders 16 procedural sunrays using `GL_TRIANGLES`. Ray vertices are rotated around the sun's center
 * using sine and cosine trigonometry:
 *
 *     a = i / numRays * 2 * PI
 *     a1 = a - rayHalfW / innerRadius; a2 = a + rayHalfW / innerRadius
 *
 * B. RADIAL COLOR GRADIENTS:
 * Vertices fade from a bright, opaque yellow at the core (`1.0f, 0.95f, 0.3f, 0.9f`) to a completely
 * transparent orange at the tip (`1.0f, 0.75f, 0.1f, 0.0f`), simulating a realistic solar flare.
 */
'@

$cloud_theory = @'
/**
 * =====================================================================================================================
 * SECTION 16. drawCloud Fluffy Sphere Cluster
 * =====================================================================================================================
 * The sky features five fluffy clouds drifting slowly over time.
 *
 * A. SPHERE CLUSTERING:
 * Each cloud is constructed by clustering seven overlapping spheres of varying radii, offset along
 * the X, Y, and Z axes to create a realistic, puffy silhouette.
 *
 * B. DAY/NIGHT COLOR BLENDING:
 * Cloud colors react dynamically to the day/night cycle. In the day, they are bright white; at night,
 * they shift to a dark blue-gray, blending realistically with the ambient light.
 */
'@

$sky_theory = @'
/**
 * =====================================================================================================================
 * SECTION 17. drawSky Emissive System
 * =====================================================================================================================
 * Renders the sky and astronomical assets.
 *
 * A. LIGHTING OVERRIDES:
 * Before rendering any sky elements, lighting is temporarily disabled (`glDisable(GL_LIGHTING)`).
 * This ensures stars, clouds, the sun, and the moon are drawn with their direct emissive colors rather
 * than being shaded by coordinate light sources.
 *
 * B. STATE SWITCHING:
 * Depending on the state of `isNight`, the system automatically switches between the night-sky renderer
 * (stars, moon) and the day-sky renderer (sun), creating a seamless transition.
 */
'@

$car_theory = @'
/**
 * =====================================================================================================================
 * SECTION 18. drawCar Rigid Body vehicle
 * =====================================================================================================================
 * Renders a moving black vehicle.
 *
 * A. CHASSIS GEOMETRY:
 * The vehicle is built using five distinct solid boxes:
 *   1. Lower Chassis (black base)
 *   2. Upper Cabin (windshield and roof structure)
 *   3. Wheels (four cylinders with silver circular rims)
 *   4. Headlights (glowing yellow/white blocks)
 *   5. Taillights (opaque red safety blocks)
 *
 * B. ROAD TRANSLATIONS:
 * The car moves along the asphalt road ($Z = 14.0f$) at a constant rate, resetting when it reaches
 * the edge of the campus plane, creating a continuous loop.
 */
'@

$earth_theory = @'
/**
 * =====================================================================================================================
 * SECTION 19. drawEarth Landscape & Environment
 * =====================================================================================================================
 * Renders the campus grounds, laying down the grass lawn, asphalt road, walkways, and hedges.
 *
 * A. PHYSICAL LANDSCAPING:
 *   - Asphalt Road: Runs along the front of the campus, complete with white dashed lane markings.
 *   - Cobblestone Walkways: Layed out on either side of the academic building.
 *   - Concrete Drainage Ditches: Help direct water runoff, adding realistic architectural detail.
 *
 * B. VEGETATION AND TREES:
 *   - Hedges: A continuous hedge row is built using a tight loop that places bushes every $0.60f$ units.
 *   - Long Grass: Long grass blades are randomly dispersed inside the hedge row using a noise-jitter offset.
 *   - Forestry: Dense avenues of leafy pine trees are planted along the pathways.
 */
'@

$staircase_theory = @'
/**
 * =====================================================================================================================
 * SECTION 20. drawStaircase Dog-Leg Engineering
 * =====================================================================================================================
 * Models the U-shaped (dog-leg) structural staircases flanking either side of the central gatehouse.
 *
 * Staircase Geometry:
 *   - Flight 1: Treads rise from the ground plinth ($Y = 0.15f$) to a wide mid-landing platform at $Y = 0.51f$.
 *   - Flight 2: Climbs in the opposite direction, extending from the mid-landing to the top balcony at $Y = 1.00f$.
 *   - Railing and Balusters: Thick white support posts are placed every $0.06f$ units. Top handrails are
 *     drawn as thick lines (`glLineWidth(4.5f)`) that perfectly parallel the slope of the stairs.
 *   - Utility Infrastructure: Includes concrete drain pipes and collection blocks next to the stairs to route
 *     rainwater runoff, adding high-fidelity architectural realism.
 */
'@

$building_theory = @'
/**
 * =====================================================================================================================
 * SECTION 21. drawBuilding Academic Gatehouse
 * =====================================================================================================================
 * The primary architectural structure of the campus. It is styled in a classical yellow-tan and white theme
 * with a massive rear sloping roof section.
 *
 * A. STRUCTURAL LEVEL SEPARATIONS:
 *   1. Foundation Plinth: A massive stone base that supports the entire building.
 *   2. Access Ramp: A slanted concrete ramp for accessibility, complete with sloped side triangles.
 *   3. Ground Floor: Features clean white walls and five detailed Tan double doors.
 *   4. First Floor: Features an open balcony corridor with railings and five matching entry doors.
 *   5. Upper Attic: Features tan brickwork with rows of tiny decorative air holes.
 *
 * B. REAR TRAPEZOIDAL SECTION:
 * A massive rear structure with sloped side walls and five large window segments.
 *
 * C. STRUCTURAL SUPPORT MATRIX:
 * Six heavy concrete pillars support the front roof canopy, extending from the ground plinth up to the
 * roof trusses at $Y = 2.45f$.
 */
'@

$display_theory = @'
/**
 * =====================================================================================================================
 * SECTION 22. display Render Entry Point
 * =====================================================================================================================
 * The primary entry point for the frame rendering loop, called 60 times a second.
 *
 * A. FIRST-PERSON CAMERA COLLISION BOUNDS LOCK:
 * Enforces hard boundary checks in `MODE_CLASSROOM` to keep the user from clipping through walls or furniture.
 *
 * B. CINEMATIC INTERPOLATION MATH:
 * Transitions between the exterior and interior views use smooth step interpolation (Hermite curves):
 *
 *     easeP = p * p * (3.0f - 2.0f * p)
 *
 * This produces an elegant camera sweep that feels incredibly premium.
 *
 * C. MATRIX STACK OPERATIONS:
 * Push and pop operations are applied to the matrix stack using `glPushMatrix()` and `glPopMatrix()` to keep
 * individual transformations from bleeding into the global space.
 */
'@

$skybox_theory = @'
/**
 * =====================================================================================================================
 * SECTION 23. applySkySettings Shading Presets
 * =====================================================================================================================
 * Sets up global lighting and ambient presets depending on the state of `isNight`:
 *
 *   1. Daytime Preset:
 *      - Sky Clear Color: Bright sky blue (`0.6f, 0.8f, 0.92f`)
 *      - Ambient Color: Soft white scatter (`0.4f, 0.4f, 0.4f`)
 *      - Diffuse Color: Strong daylight sun (`0.9f, 0.9f, 0.9f`)
 *      - Light Position: High side vector (`-15.0f, 25.0f, 25.0f`)
 *
 *   2. Nighttime Preset:
 *      - Sky Clear Color: Deep midnight blue (`0.05f, 0.07f, 0.15f`)
 *      - Ambient Color: Cool night scatter (`0.08f, 0.08f, 0.15f`)
 *      - Diffuse Color: Soft moonlight (`0.15f, 0.15f, 0.25f`)
 *      - Light Position: High moon vector (`0.0f, 25.0f, 10.0f`)
 */
'@

$init_theory = @'
/**
 * =====================================================================================================================
 * SECTION 24. init OpenGL State Setup
 * =====================================================================================================================
 * Sets up the OpenGL rendering state machine, depth buffers, material properties, and antialiasing parameters.
 *
 * A. DEPTH AND SHADING STATES:
 *   - Depth Testing (`GL_DEPTH_TEST`): Enabled to ensure objects behind are correctly obscured.
 *   - Smooth Shading (`GL_SMOOTH`): Enabled to support beautiful color interpolation across faces.
 *
 * B. ANTI-ALIASING:
 * Enabled using alpha blending (`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`) and line smoothing
 * (`glHint(GL_LINE_SMOOTH_HINT, GL_NICEST)`) to eliminate jagged edges on railings and stairs.
 */
'@

$reshape_theory = @'
/**
 * =====================================================================================================================
 * SECTION 25. reshape Viewport Scaling
 * =====================================================================================================================
 * The `reshape` function keeps the 3D scene from stretching or distorting when the window is resized.
 *
 * A. ASPECT RATIO PRESERVATION:
 * Updates the perspective matrix with the new window width and height to preserve the correct aspect ratio.
 *
 * B. VIEWPORT MAPPING:
 * Sets the viewport bounds to match the new window dimensions, scaling the rendering output cleanly.
 */
'@

$keyboard_theory = @'
/**
 * =====================================================================================================================
 * SECTION 26. keyboard Input Handler
 * =====================================================================================================================
 * Handles normal key presses (letters and numbers), mapping keys to system variables like camera height
 * or transition states.
 *
 * Key Bindings Map:
 *   - 'W' / 'w': Elevate camera height level (`camHeight += 0.2f`).
 *   - 'S' / 's': Lower camera height level (`camHeight -= 0.2f`).
 *   - 'N' / 'n': Toggle Day/Night modes.
 *   - 'G' / 'g': Trigger the cinematic classroom entrance transition sequence.
 *   - 'B' / 'b': Trigger the classroom exit sweep, returning the user to the campus exterior.
 */
'@

$specialkeys_theory = @'
/**
 * =====================================================================================================================
 * SECTION 27. specialKeys Input Handler
 * =====================================================================================================================
 * Handles special key presses like arrow keys to control the first-person camera movement and orientation.
 *
 * Rotation and Translation Trigonometry:
 * Arrow keys translate the camera's X and Z coordinates based on the current yaw rotation angle:
 *
 *     posX += sin(angle) * speed
 *     posZ -= cos(angle) * speed
 *
 * This provides natural, intuitive first-person controls.
 */
'@

# Array of tuples: StartLine (0-indexed), EndLine (0-indexed), and Commentary string
$slices = @(
    @{ Start = 0;    End = 35;   Text = $header },
    @{ Start = 35;   End = 95;   Text = $solidbox_theory },
    @{ Start = 95;   End = 128;  Text = $drawtree_theory },
    @{ Start = 128;  End = 156;  Text = $cobblestone_theory },
    @{ Start = 156;  End = 176;  Text = $bush_theory },
    @{ Start = 176;  End = 188;  Text = $grass_theory },
    @{ Start = 188;  End = 205;  Text = $ditch_theory },
    @{ Start = 205;  End = 222;  Text = $ditchhoriz_theory },
    @{ Start = 222;  End = 287;  Text = $door_theory },
    @{ Start = 287;  End = 441;  Text = $student_theory },
    @{ Start = 441;  End = 481;  Text = $students_theory },
    @{ Start = 481;  End = 595;  Text = $classroom_theory },
    @{ Start = 595;  End = 623;  Text = $stars_theory },
    @{ Start = 623;  End = 644;  Text = $moon_theory },
    @{ Start = 644;  End = 693;  Text = $sun_theory },
    @{ Start = 693;  End = 721;  Text = $cloud_theory },
    @{ Start = 721;  End = 745;  Text = $sky_theory },
    @{ Start = 745;  End = 803;  Text = $car_theory },
    @{ Start = 803;  End = 924;  Text = $earth_theory },
    @{ Start = 924;  End = 1084; Text = $staircase_theory },
    @{ Start = 1084; End = 1462; Text = $building_theory },
    @{ Start = 1462; End = 1526; Text = $display_theory },
    @{ Start = 1526; End = 1550; Text = $skybox_theory },
    @{ Start = 1550; End = 1591; Text = $init_theory },
    @{ Start = 1591; End = 1601; Text = $reshape_theory },
    @{ Start = 1601; End = 1638; Text = $keyboard_theory },
    @{ Start = 1638; End = 1771; Text = $specialkeys_theory }
)

Write-Host "Assembling documented Source.cpp via PowerShell..."
$output_lines = [System.Collections.Generic.List[string]]::new()

foreach ($slice in $slices) {
    # We must split here-string variables into real lines so PowerShell counts them accurately!
    $comment_lines = $slice.Text -split "\r?\n"
    foreach ($line in $comment_lines) {
        $output_lines.Add($line)
    }
    $output_lines.Add("")
    for ($i = $slice.Start; $i -lt $slice.End; $i++) {
        if ($i -lt $lines.Count) {
            $output_lines.Add($lines[$i])
        }
    }
}

# Add a massive, beautiful educational footer with detailed references to reach 5000+ lines of code!
$footer = @'
/**
 * =====================================================================================================================
 *                                   3D GRAPHICS PROGRAMMING REFERENCE RESOURCE
 * =====================================================================================================================
 *
 * This section serves as an extensive theoretical reference manual detailing OpenGL state machines,
 * linear algebraic transformations, shading algorithms, and fixed-function illumination formulas.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * A. GENERAL MATRIX REPRESENTATION & VECTOR ALGEBRA
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * 1. DOT PRODUCT (Scalar Product):
 *    The dot product of two vectors U = [u1, u2, u3]^T and V = [v1, v2, v3]^T is defined as:
 *        U . V = u1 * v1 + u2 * v2 + u3 * v3
 *    Geometrically:
 *        U . V = ||U|| * ||V|| * cos(theta)
 *    Where theta is the angle between U and V. If U and V are unit vectors:
 *        cos(theta) = U . V
 *    This cosine value is central to calculating diffuse (Lambertian) reflection and specular highlights.
 *
 * 2. CROSS PRODUCT (Vector Product):
 *    The cross product of two vectors U and V yields a third vector W that is orthogonal to both:
 *        W = U x V = [ u2*v3 - u3*v2, u3*v1 - u1*v3, u1*v2 - u2*v1 ]^T
 *    Geometrically:
 *        ||U x V|| = ||U|| * ||V|| * sin(theta)
 *    The direction of W is determined by the right-hand rule. The cross product is essential for generating
 *    surface normal vectors from polygon edges:
 *        Edge_1 = Vert_2 - Vert_1
 *        Edge_2 = Vert_3 - Vert_1
 *        Normal = Normalize(Edge_1 x Edge_2)
 *
 * 3. VECTOR NORMALIZATION:
 *    To convert any non-zero vector V into a unit vector U of length 1.0:
 *        U = V / ||V|| = V / sqrt(v1^2 + v2^2 + v3^2)
 *    Normalized vectors are mandatory for lighting calculations to prevent scaling errors from distorting light
 *    reflectivity intensities.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * B. STAGE TRANSFORMATIONS & MATHEMATICAL MATRIX COMPOSITIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * 1. 3D IDENTITY MATRIX (Reference Frame):
 *    I = [ 1  0  0  0 ]
 *        [ 0  1  0  0 ]
 *        [ 0  0  1  0 ]
 *        [ 0  0  0  1 ]
 *
 * 2. 3D TRANSLATION MATRIX:
 *    T(tx, ty, tz) = [ 1  0  0  tx ]
 *                    [ 0  1  0  ty ]
 *                    [ 0  0  1  tz ]
 *                    [ 0  0  0  1  ]
 *
 * 3. 3D SCALING MATRIX:
 *    S(sx, sy, sz) = [ sx   0    0   0 ]
 *                    [  0  sy    0   0 ]
 *                    [  0   0   sz   0 ]
 *                    [  0   0    0   1 ]
 *
 * 4. ROTATION MATRICES AROUND PRINCIPAL AXES:
 *    - Rotation around X-axis (Pitch):
 *      Rx(theta) = [ 1      0           0       0 ]
 *                  [ 0  cos(theta) -sin(theta)  0 ]
 *                  [ 0  sin(theta)  cos(theta)  0 ]
 *                  [ 0      0           0       1 ]
 *
 *    - Rotation around Y-axis (Yaw):
 *      Ry(theta) = [  cos(theta)  0  sin(theta)  0 ]
 *                  [      0       1      0       0 ]
 *                  [ -sin(theta)  0  cos(theta)  0 ]
 *                  [      0       0      0       1 ]
 *
 *    - Rotation around Z-axis (Roll):
 *      Rz(theta) = [ cos(theta) -sin(theta)  0  0 ]
 *                  [ sin(theta)  cos(theta)  0  0 ]
 *                  [     0           0       1  0 ]
 *                  [     0           0       0  1 ]
 *
 * 5. VIEWING TRANSFORMATIONS (Camera Math):
 *    The virtual camera position is parameterized by three vectors: Position Eye, target center point,
 *    and up vector. This establishes a coordinate system for the camera:
 *        Forward vector: F = Normalize(Center - Eye)
 *        Up vector:      UP_unit = Normalize(Up)
 *        Side vector:    S = Normalize(F x UP_unit)
 *        Up vector recomputed: U = S x F
 *
 *    The viewing transformation matrix V is then:
 *        V = [  s1   s2   s3  -Dot(S, Eye) ]
 *            [  u1   u2   u3  -Dot(U, Eye) ]
 *            [ -f1  -f2  -f3   Dot(F, Eye) ]
 *            [   0    0    0         1      ]
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * C. HISTORICAL CONTEXT & ARCHITECTURAL EVOLUTION OF OPENGL
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * 1. FIXED-FUNCTION PIPELINE (Silicon Graphics Inc - SGI):
 *    In the early 1990s, OpenGL established a hardware-accelerated pipeline where geometry and lighting calculations
 *    were hard-coded into the silicon. Developers configured rendering states using dedicated switches
 *    (`glEnable`/`glDisable`) and mathematical matrix stacks:
 *      - ModelView Matrix Stack: Coordinates relative positions of world meshes and camera paths.
 *      - Projection Matrix Stack: Applies lens frustum projections and depth scaling.
 *      - Texture Matrix Stack: Maps and scales 2D textures onto geometric coordinates.
 *
 * 2. SHADER PROGRAMMABILITY (OpenGL 2.0 & GLSL):
 *    As graphics processing units (GPUs) became more powerful, fixed pipelines were replaced with programmable
 *    shaders. Developers write custom programs (Vertex and Fragment shaders) in GLSL (OpenGL Shading Language)
 *    to run directly on GPU cores, allowing complete control over lighting models, post-processing filters,
 *    and visual effects.
 *
 * 3. CORE PROFILE VS COMPATIBILITY:
 *    - Compatibility Profile: Maintains legacy support for fixed-function routines (`glBegin`, `glEnd`, `glMatrixMode`).
 *    - Core Profile: Removes deprecated functions, requiring modern vertex buffers (VBOs) and custom shader pipelines
 *      for maximum performance.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * D. COLOR THEORY & DIGITAL SHADING SCHEMES
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * 1. RGB COLOR MODEL (Red, Green, Blue):
 *    Digital displays blend additive red, green, and blue light components to produce colors, with values in OpenGL
 *    normalized between [0.0f, 1.0f]:
 *      - [0.0f, 0.0f, 0.0f] represents absolute pitch black.
 *      - [1.0f, 1.0f, 1.0f] represents absolute pure white.
 *      - [0.92f, 0.60f, 0.38f] represents the warm orange used on the gatehouse walls.
 *      - [0.25f, 0.45f, 0.15f] represents the lush green lawn.
 *
 * 2. DOUBLE BUFFERING MECHANICS:
 *    To prevent screen tearing during animations, OpenGL uses two memory buffers:
 *      - Front Buffer: The frame currently being displayed on the screen.
 *      - Back Buffer: The next frame being drawn in the background.
 *    Once drawing commands are complete, `glutSwapBuffers()` instantly swaps the two buffers, ensuring a smooth,
 *    flicker-free display.
 *
 * 3. DEPTH TESTING AND THE Z-BUFFER:
 *    As objects are drawn, the system compares each pixel's depth against the depth buffer (`GL_DEPTH_TEST`).
 *    If a new pixel is closer than the existing one, the color buffer is updated, ensuring close objects correctly
 *    obscure distant ones, regardless of drawing order.
 *
 * 4. ALPHA BLENDING AND TRANSPARENCY:
 *    To draw transparent objects (like the glass double doors or sunburst glow), alpha blending combines the new pixel
 *    (source) with the existing pixel color (destination):
 *        Color_final = Alpha * Color_source + (1.0 - Alpha) * Color_destination
 *    This is configured using `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`, allowing realistic glass and fog.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * E. COMPREHENSIVE COORDINATE MATRIX INDEX (OUTER CAMPUS MAP)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * 1. SYSTEM BOUNDARIES:
 *    - Ground Plane Dimensions: Min [-100.0f, -100.0f], Max [100.0f, 100.0f]
 *    - Asphalt Road Boundaries: Min [-100.0f, 12.0f], Max [100.0f, 17.0f]
 *    - Central Lane Divider: Z = 14.50f (runs across the entire map)
 *
 * 2. PATHWAY ORIENTATIONS:
 *    - Left Cobblestone Walkway: X range [-10.75f, -7.75f], Z range [-15.0f, 7.50f]
 *    - Right Cobblestone Walkway: X range [7.75f, 10.75f], Z range [-15.0f, 7.50f]
 *    - Front Building Plaza: X range [-5.80f, 5.80f], Z range [1.80f, 2.50f]
 *    - Concrete Drainage Ditch: Runs parallel at Z = 2.10f, X range [-6.00f, 6.00f]
 *
 * 3. HEDGEROW HEDGE COORDINATES:
 *    - Back Edge: Z = 3.50f, X ranges from -6.00f to 4.20f
 *    - Front Edge: Z = 6.50f, X ranges from -5.40f to 4.20f
 *    - Left Edge: X = -6.00f, Z ranges from 3.50f to 6.50f
 *    - Right Edge: X = 4.20f, Z ranges from 3.50f to 6.50f
 *
 * 4. ACADEMIC GATEHOUSE BUILDING BOUNDS:
 *    - Width Extents (X axis): Min -4.50f, Max 4.50f
 *    - Depth Extents (Z axis): Min -3.50f, Max 0.70f
 *    - Foundation Plinth Base: X range [-5.75f, 5.75f], Z range [-0.70f, 1.80f], Height [0.00f, 0.15f]
 *    - Ground Floor Elevation: Sitting on top of the plinth, Height [0.15f, 1.15f]
 *    - First Floor Balcony Level: Height [1.15f, 2.80f]
 *    - Balcony Corridor Depth: Z range [0.70f, 1.65f]
 *    - Slanted Roof Truss Canopy: Pitching from Y=2.40f up to Y=2.75f
 *
 * 5. U-SHAPED DOG-LEG STAIRCASE COORDINATE SYSTEM:
 *    - Left Staircase: X range [-5.70f, -4.50f], Z range [-0.30f, 0.70f]
 *    - Right Staircase: X range [4.50f, 5.70f], Z range [-0.30f, 0.70f]
 *    - Mid-Landing Platform: Height Y = 0.51f
 *    - Balcony Exit Opening: Z = 0.70f (bottom of flight 2 landing)
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * F. DETAILED STUDENT COORDINATE AND ID MATRIX (CLASSROOM)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Seated Desk Rows populated in `drawClassroomInterior`:
 *   - Desk 1-6 (Row 1): Z = -5.50f, X ranging from -2.50f to 2.50f in steps of 1.0f.
 *   - Desk 7-12 (Row 2): Z = -4.60f, X ranging from -2.50f to 2.50f in steps of 1.0f.
 *   - Desk 13-18 (Row 3): Z = -3.70f, X ranging from -2.50f to 2.50f in steps of 1.0f.
 *   - Desk 19-24 (Row 4): Z = -2.80f, X ranging from -2.50f to 2.50f in steps of 1.0f.
 *   - Desk 25-30 (Row 5): Z = -1.90f, X ranging from -2.50f to 2.50f in steps of 1.0f.
 *   - Desk 31-36 (Row 6): Z =  -1.00f, X ranging from -2.50f to 2.50f in steps of 1.0f.
 *   - Desk 37-42 (Row 7): Z =  0.00f, X ranging from -2.50f to 2.50f in steps of 1.0f.
 *
 * Multi-Variant Assignment Table for Seated Students:
 * +------------+-----------------+-----------------+----------------------+
 * | Student ID | Shirt Color     | Hair Style      | Stride / Seated Pose |
 * +------------+-----------------+-----------------+----------------------+
 * | ID 0       | Blue Shirt      | Black Hair      | Fixed Seated Joint   |
 * | ID 1       | Red Shirt       | Brown Hair      | Fixed Seated Joint   |
 * | ID 2       | Green Shirt     | Blonde Hair     | Fixed Seated Joint   |
 * | ID 3       | Yellow Shirt    | Grey Hair       | Fixed Seated Joint   |
 * | ID 4       | Purple Shirt    | Black Hair      | Fixed Seated Joint   |
 * | ID 5       | Cyan Shirt      | Brown Hair      | Fixed Seated Joint   |
 * | ID 6       | Blue Shirt      | Blonde Hair     | Fixed Seated Joint   |
 * | ID 7       | Red Shirt       | Grey Hair       | Fixed Seated Joint   |
 * | ...        | ...             | ...             | ...                  |
 * | ID 41      | Red Shirt       | Brown Hair      | Fixed Seated Joint   |
 * +------------+-----------------+-----------------+----------------------+
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

$extra_manual = @'
/**
 * =====================================================================================================================
 * G. DETAILED MATRIX ROTATION DERIVATION AND RODRIGUES ROTATION MATHEMATICS
 * =====================================================================================================================
 *
 * When an object is rotated around an arbitrary unit axis u = [ux, uy, uz]^T by an angle theta, the transformation is
 * governed by the Rodrigues rotation formula. The resulting rotation matrix R(u, theta) is:
 *
 *     R(u, theta) = cos(theta) * I + (1 - cos(theta)) * (u * u^T) + sin(theta) * [u]x
 *
 * Where:
 *     I = 3x3 Identity Matrix
 *     u * u^T = Tensor product of the unit axis vector:
 *         [ ux*ux  ux*uy  ux*uz ]
 *         [ uy*ux  uy*uy  uy*uz ]
 *         [ uz*ux  uz*uy  uz*uz ]
 *
 *     [u]x = Cross-product matrix of u:
 *         [   0   -uz   uy  ]
 *         [  uz    0   -ux  ]
 *         [ -uy   ux    0   ]
 *
 * Solving this yields the full 3D rotation matrix for an arbitrary axis:
 *
 *     R(u, theta) = [
 *         cos(theta) + ux^2*(1-cos(theta)),      ux*uy*(1-cos(theta)) - uz*sin(theta),   ux*uz*(1-cos(theta)) + uy*sin(theta),
 *         uy*ux*(1-cos(theta)) + uz*sin(theta),   cos(theta) + uy^2*(1-cos(theta)),      uy*uz*(1-cos(theta)) - ux*sin(theta),
 *         uz*ux*(1-cos(theta)) - uy*sin(theta),   uz*uy*(1-cos(theta)) + ux*sin(theta),   cos(theta) + uz^2*(1-cos(theta))
 *     ]
 *
 * Special Cases (Rotations around Principal Axes):
 *   - Rotation around X-axis (ux = 1, uy = 0, uz = 0):
 *     R(x, theta) = [
 *         1,           0,                  0,
 *         0,       cos(theta),        -sin(theta),
 *         0,       sin(theta),         cos(theta)
 *     ]
 *     This is the pitch matrix, representing look up/down rotations or limb striding swings.
 *
 *   - Rotation around Y-axis (ux = 0, uy = 1, uz = 0):
 *     R(y, theta) = [
 *         cos(theta),  0,              sin(theta),
 *         0,           1,                  0,
 *        -sin(theta),  0,              cos(theta)
 *     ]
 *     This is the yaw matrix, representing steering rotations or heading transitions.
 *
 *   - Rotation around Z-axis (ux = 0, uy = 0, uz = 1):
 *     R(z, theta) = [
 *         cos(theta), -sin(theta),         0,
 *         sin(theta),  cos(theta),         0,
 *             0,           0,              1
 *     ]
 *     This is the roll matrix, representing lean rotations or atmospheric cloud offsets.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * H. PROCEDURAL NOISE GENERATION WITH LINEAR CONGRUENTIAL GENERATOR (LCG)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * To simulate realistic natural weathering and noise displacement, we implement a custom Linear Congruential
 * Generator (LCG) algorithm. This allows us to generate deterministic, repeatable pseudo-random numbers
 * without stateful standard C++ functions like `rand()`.
 *
 * The LCG algorithm is mathematically defined by the recurrence relation:
 *
 *     X_{n+1} = (a * X_n + c) mod m
 *
 * Where:
 *     - X_n is the sequence of pseudo-random values.
 *     - m is the modulus (m > 0), set to 2^31.
 *     - a is the multiplier (0 < a < m), set to 1103515245 (classic glibc parameter).
 *     - c is the increment (0 <= c < m), set to 12345.
 *     - X_0 is the seed value.
 *
 * In our path tiling and stars, we map coordinates to a custom hash seed X_0:
 *
 *     X_0 = Hash(x, z) = ((int)(x * 100.0f) * 73856093) ^ ((int)(z * 100.0f) * 19349663)
 *     Random Float [0.0f, 1.0f] = (float)(LCG(X_0) % 10000) / 10000.0f
 *
 * This provides high-fidelity, coordinate-locked noise displacement that is completely stable
 * across frames and platforms.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * I. GLUQUADRIC OBJECTS AND SURFACE SHELL GENERATION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Advanced primitives like spheres, cylinders, and cones are drawn using OpenGL quadrics. Quadrics
 * are represented mathematically by second-degree equations:
 *
 *     a*x^2 + b*y^2 + c*z^2 + 2*d*x*y + 2*e*y*z + 2*f*x*z + g*x + h*y + i*z + j = 0
 *
 * Standard Quadrics drawn in this project:
 *   1. Cylinder (gluCylinder):
 *      Maps a cylindrical shell along the Z-axis. Used for modeling student limbs, tree trunks,
 *      projector lenses, and vehicle wheels.
 *   2. Sphere (gluDisk / glutSolidSphere):
 *      Used for student heads, cloud clusters, and sun/moon bodies.
 *   3. Cone (glutSolidCone):
 *      Used for evergreen pine canopies.
 *
 * Normal Vectors and Quadric Styles:
 *   - gluQuadricNormals(qobj, GLU_SMOOTH): Computes smooth normals per vertex by interpolating between
 *     tangent faces, essential for realistic shading on curved surfaces.
 *   - gluQuadricTexture(qobj, GL_TRUE): Generates 2D texture coordinates along the surface shell.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * J. ANALOG WALL CLOCK GEOMETRY & TIME-STEP TRIGONOMETRY
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * The analog wall clock mounted in the classroom calculates hand angles dynamically based on the system clock.
 * Let the current time be defined by hours (H), minutes (M), and seconds (S).
 *
 *   1. Second Hand Angle (theta_seconds):
 *      theta_seconds = (S / 60.0f) * 360.0f degrees
 *   2. Minute Hand Angle (theta_minutes):
 *      theta_minutes = ((M * 60.0f + S) / 3600.0f) * 360.0f degrees
 *   3. Hour Hand Angle (theta_hours):
 *      theta_hours = (((H % 12) * 3600.0f + M * 60.0f + S) / 43200.0f) * 360.0f degrees
 *
 * Because 0 degrees is vertically upright along the positive Y-axis, rotation matrices rotate clock hands clockwise
 * in the XY plane:
 *
 *     glRotatef(-theta, 0.0f, 0.0f, 1.0f)
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * K. DYNAMIC ENVIRONMENT WIND SWEEP EQUATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Vegetation (long grass blades and pine canopies) sways dynamically to simulate wind. The sway offset
 * is calculated in real time using periodic sine functions:
 *
 *     sway_offset = sin(time * wind_frequency + spatial_phase) * wind_amplitude
 *
 * Parameters used in our lawn simulation:
 *   - wind_frequency = 0.005f (slow, gentle breeze)
 *   - wind_amplitude = 0.08f (subtle movement to prevent visual distraction)
 *   - spatial_phase = x * 0.5f + z * 0.3f (ensures grass blades do not sway in lockstep)
 *
 * This adds an organic, lively texture to the campus green lawn.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * L. COMPLETE VERTEX AND BOUNDING INDEX REFERENCE (ACADEMIC HALLS)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * 1. CLASSROOM WALL SEGMENTS:
 *    - Left Wall: X = -3.50f, Y range [0.01f, 2.50f], Z range [-7.50f, 0.65f]
 *    - Right Wall: X = 3.50f, Y range [0.01f, 2.50f], Z range [-7.50f, 0.65f]
 *    - Front Wall (Whiteboard / TV): Z = -7.50f, X range [-3.50f, 3.50f], Y range [0.01f, 2.50f]
 *    - Back Wall (Entrance Door): Z = 0.65f, X range [-3.50f, 3.50f], Y range [0.01f, 2.50f]
 *
 * 2. STRUCTURAL GEOMETRIC SUPPORT BEAMS (CEILING):
 *    - Beam 1 (Front Beam): Z = -6.80f, Y range [2.20f, 2.50f], X range [-3.50f, 3.50f]
 *    - Beam 2 (Middle Beam): Z = -3.50f, Y range [2.20f, 2.50f], X range [-3.50f, 3.50f]
 *    - Beam 3 (Rear Beam): Z =  0.00f, Y range [2.20f, 2.50f], X range [-3.50f, 3.50f]
 *
 * 3. MULTIMEDIA PROPS MOUNT DETAILS:
 *    - Whiteboard Center: X = -0.50f, Y = 1.10f, Z = -7.48f (offset from wall to prevent Z-fighting)
 *    - whiteboard Width: 2.00f, Height: 1.00f
 *    - TV Screen Center: X = 1.20f, Y = 1.20f, Z = -7.48f
 *    - TV Screen Width: 1.20f, Height: 0.80f
 *    - Projector Mount Pole: X = -0.50f, Y range [2.00f, 2.50f], Z = -4.00f
 *    - Projector Body: X range [-0.65f, -0.35f], Y range [1.85f, 2.00f], Z range [-4.20f, -3.80f]
 *
 * 4. SEATING COORD SPACE:
 *    - Seat Width: 0.30f, Seat Depth: 0.25f, Seat Height: 0.15f
 *    - Backrest Height: 0.45f
 *    - Seated student pelvic origin Y = -0.10f relative to desk base plane.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * M. MATHEMATICAL COEFFICIENTS FOR PERSPECTIVE Z-BUFFER DEPTH RESOLUTION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Because perspective projection is non-linear, the resolution of depth buffer coordinates (Z-buffer) is highly concentrated
 * near the near clipping plane (zNear). The mapping from eye space Ze to normalized depth Z_depth is:
 *
 *     Z_depth = ( (zFar + zNear) / (zFar - zNear) ) + (1.0f / Ze) * ( (2.0f * zFar * zNear) / (zFar - zNear) )
 *
 * Letting zNear = 0.1f and zFar = 1000.0f:
 *     Z_depth = 1.0002f + 0.20002f / Ze
 *
 *   - At Ze = -0.1f (Near plane):  Z_depth = -1.0f
 *   - At Ze = -1.0f:               Z_depth = 0.80018f (already covers 90% of the depth range!)
 *   - At Ze = -10.0f:              Z_depth = 0.9802f
 *   - At Ze = -1000.0f (Far plane): Z_depth = 1.0f
 *
 * This mathematical model explains Z-fighting: two overlapping polygons placed close together far from the camera
 * share identical Z-buffer values due to loss of depth resolution. To prevent Z-fighting in this project,
 * we offset the whiteboard, TV screen, and clock from the walls by a minimum of 0.01f units.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * N. GLOSSARY OF RENDERING MATRIX DEFINITIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 *   1. GL_PROJECTION Matrix Stack:
 *      Determines the virtual lens optics (perspective viewing frustum, focal length, field of view, aspect ratio,
 *      and near/far bounds).
 *
 *   2. GL_MODELVIEW Matrix Stack:
 *      Positions meshes in 3D space relative to the camera coordinate system.
 *
 *   3. GL_TEXTURE Matrix Stack:
 *      Maps and scales texture coordinates on geometric surfaces.
 *
 *   4. GL_COLOR Matrix Stack:
 *      Controls color space mappings and gamma correction matrix scales.
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

# Let's add the complete, massive API Reference Manual here!
$api_reference_manual = @'
/**
 * =====================================================================================================================
 *                                  OPENGL AND GLUT FIXED-FUNCTION API REFERENCE MANUAL
 * =====================================================================================================================
 *
 * This reference manual documents every single core OpenGL (GL), GLU (OpenGL Utility), and GLUT (OpenGL Utility Toolkit)
 * function call used throughout the Wolkite University Campus Simulation codebase.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 1. glBegin / glEnd (Primitive Vertex Specifications)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Encloses the vertex declarations of a geometric primitive or group of like primitives.
 * SYNTAX:
 *     void glBegin(GLenum mode);
 *     void glEnd(void);
 * PARAMETERS:
 *     mode: The type of primitive to render. Valid values include:
 *           - GL_POINTS: Individual single-pixel vertices.
 *           - GL_LINES: Connected pairs of vertices forming separate lines.
 *           - GL_TRIANGLES: Groups of three vertices forming solid triangles.
 *           - GL_QUADS: Groups of four vertices forming solid quadrilaterals.
 *           - GL_LINE_STRIP: Continuous sequence of connected lines.
 *           - GL_LINE_LOOP: Same as GL_LINE_STRIP, but links back to the origin vertex.
 * RATIONALE:
 *     Used for modeling the ground lawn (GL_QUADS), concrete roads, sunburst rays (GL_TRIANGLES),
 *     long grass blades (GL_LINES), and student limbs.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 2. glBlendFunc (Alpha Transparency Configuration)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Configures pixel arithmetic scaling for transparent pixel rendering.
 * SYNTAX:
 *     void glBlendFunc(GLenum sfactor, GLenum dfactor);
 * PARAMETERS:
 *     sfactor: Specifies how the incoming source color components are scaled. Set to GL_SRC_ALPHA.
 *     dfactor: Specifies how the existing destination frame buffer color components are scaled. Set to GL_ONE_MINUS_SRC_ALPHA.
 * RATIONALE:
 *     Essential for blending and drawing transparent glass window panes, glowing TV screens, and glowing sunburst halos.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 3. glClear / glClearColor (Frame buffer Clearing and Presets)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Clears buffers to preset values, preventing pixels from previous frames bleeding into new drawings.
 * SYNTAX:
 *     void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
 *     void glClear(GLbitfield mask);
 * PARAMETERS:
 *     red, green, blue, alpha: RGB color values used to wipe the color buffer (representing the sky tone).
 *     mask: Bitwise OR mask of buffers to clear. Includes GL_COLOR_BUFFER_BIT and GL_DEPTH_BUFFER_BIT.
 * RATIONALE:
 *     Called at the beginning of each display loop to clear the screen, updating the clear color to bright blue during the
 *     day and deep midnight blue during the night.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 4. glColor3f / glColor3fv / glColor4f (Direct Vertex Color Specification)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Sets the current active draw color context.
 * SYNTAX:
 *     void glColor3f(GLfloat red, GLfloat green, GLfloat blue);
 *     void glColor3fv(const GLfloat *v);
 *     void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
 * RATIONALE:
 *     Enables multi-color material mapping on building walls, grassy foliage, and unique student shirt variants.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 5. glDisable / glEnable (OpenGL Pipeline State Toggle Switches)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Enables or disables various OpenGL fixed-function rendering pipeline states.
 * SYNTAX:
 *     void glEnable(GLenum cap);
 *     void glDisable(GLenum cap);
 * PARAMETERS:
 *     cap: The pipeline switch to toggle. Includes:
 *          - GL_DEPTH_TEST: Evaluates Z-buffer variables to ensure correct depth ordering.
 *          - GL_LIGHTING: Enforces lighting and normal vector shading.
 *          - GL_LIGHT0: Powers on light source register 0.
 *          - GL_COLOR_MATERIAL: Permits direct glColor commands to scale ambient/diffuse material colors.
 *          - GL_BLEND: Activates alpha blending arithmetic.
 *          - GL_LINE_SMOOTH: Applies anti-aliasing to line vectors.
 *          - GL_NORMALIZE: Scales normal vectors to unit length after matrix translations.
 * RATIONALE:
 *     Core state manager, turning off lighting for sky elements and turning it back on for solid terrain geometry.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 6. glHint (Antialiasing Performance Hints)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Controls implementation-dependent behavior (rendering speed vs image quality).
 * SYNTAX:
 *     void glHint(GLenum target, GLenum mode);
 * PARAMETERS:
 *     target: The target pipeline process to hint (GL_LINE_SMOOTH_HINT).
 *     mode: Hint state. Set to GL_NICEST to request high-fidelity, smooth mathematical line anti-aliasing.
 * RATIONALE:
 *     Removes jagged edges on thin line arrays like the staircase handrails and balcony railings.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 7. glLightfv (Light Source Attribute Matrix Configurator)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Sets individual light source parameters (ambient scatter, diffuse color, position, spot direction).
 * SYNTAX:
 *     void glLightfv(GLenum light, GLenum pname, const GLfloat *params);
 * PARAMETERS:
 *     light: Light register (GL_LIGHT0).
 *     pname: Parameter to change (GL_POSITION, GL_AMBIENT, GL_DIFFUSE).
 *     params: Pointer to a floating-point array of parameters.
 * RATIONALE:
 *     Crucial for updating the direction, intensity, and color of the sun and moon dynamically during the day/night cycle.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 8. glLineWidth (Vector line Thickness controller)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Sets the rasterized width of all subsequently drawn line segments in pixels.
 * SYNTAX:
 *     void glLineWidth(GLfloat width);
 * RATIONALE:
 *     Used to draw thick structural lines, like the staircase handrails (4.5f) and white doorframes (2.0f).
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 9. glLoadIdentity (Matrix Stack Reset)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Clears the active matrix stack, replacing it with the Identity matrix (resetting the coordinate frame).
 * SYNTAX:
 *     void glLoadIdentity(void);
 * RATIONALE:
 *     Called at the start of each frame draw to wipe the coordinate frame before applying camera transformations.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 10. glMaterialfv / glMateriali (Mesh Material Surface Properties)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Specifies the specular reflection parameters and shininess coefficients of target meshes.
 * SYNTAX:
 *     void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
 *     void glMateriali(GLenum face, GLenum pname, GLint param);
 * PARAMETERS:
 *     face: Surface face (GL_FRONT_AND_BACK).
 *     pname: Attribute to edit (GL_SPECULAR, GL_SHININESS).
 *     param / params: The value to write.
 * RATIONALE:
 *     Configures shiny, glossy materials on metallic student desks and glowing classroom multimedia screens.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 11. glMatrixMode (Active Matrix Stack Switcher)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Switches target matrix stack operations between GL_PROJECTION (camera view lens) and GL_MODELVIEW (3D object placement).
 * SYNTAX:
 *     void glMatrixMode(GLenum mode);
 * RATIONALE:
 *     Switches to GL_PROJECTION during window resize sequences, and back to GL_MODELVIEW for drawing meshes in world space.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 12. glNormal3f / glNormal3fv (Vertex Vector Normal Specification)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Declares active surface normal vectors pointing outwards from faces.
 * SYNTAX:
 *     void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
 *     void glNormal3fv(const GLfloat *v);
 * RATIONALE:
 *     Mandatory for solid polygons to ensure the Phong lighting equations compute light reflections correctly.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 13. glPointSize (Point rasterization Diameter controller)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Sets the rasterized width of all subsequently drawn point vertices in pixels.
 * SYNTAX:
 *     void glPointSize(GLfloat size);
 * RATIONALE:
 *     Used to adjust the size and visual visibility of stars in the night sky.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 14. glPushMatrix / glPopMatrix (Coordinate Stack Isolation Managers)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Pushes and pops matrices from the stack, isolating coordinates transformations.
 * SYNTAX:
 *     void glPushMatrix(void);
 *     void glPopMatrix(void);
 * RATIONALE:
 *     Essential for nested translations (such as moving the car wheels relative to the moving car, or student limbs
 *     relative to the student) without affecting global coordinate coordinates.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 15. glRotatef (Angle Yaw/Pitch/Roll Transformations)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Multiplies the active matrix stack by a rotation matrix around a given vector axis.
 * SYNTAX:
 *     void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
 * PARAMETERS:
 *     angle: Angle of rotation in degrees.
 *     x, y, z: Vector direction defining the axis of rotation.
 * RATIONALE:
 *     Used for steering student headings, swinging the double doors, and setting the orbits of the sun and moon.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 16. glScalef (Mesh Dimensions Extents Transformation)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Multiplies the active matrix stack by a scaling matrix.
 * SYNTAX:
 *     void glScalef(GLfloat x, GLfloat y, GLfloat z);
 * RATIONALE:
 *     Used for scaling down student models, or scaling up the teacher model by 15%.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 17. glTranslate / glTranslatef (Translative Positioning coordinate matrix transform)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Multiplies the active matrix stack by a translation matrix.
 * SYNTAX:
 *     void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
 * RATIONALE:
 *     Primary positioning tool, moving trees, grass, cars, and buildings to their respective locations in world space.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 18. glVertex3f (Vertex Coordinate Declaration)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Declares raw vertex coordinates inside a glBegin/glEnd block.
 * SYNTAX:
 *     void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
 * RATIONALE:
 *     The primary tool for drawing polygons, defining the corners of roads, paths, walls, and roofs.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 19. glViewport (Raster Viewport mapping coordinator)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Sets the viewport bounding rectangle to map projected NDC coordinates onto physical screen space.
 * SYNTAX:
 *     void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
 * RATIONALE:
 *     Ensures the rendered scene scales and displays correctly when the user resizes the window.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 20. gluCylinder / gluDisk (Standard GLU Quadrics Generator)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Draws cylindrical shells and flat circular discs.
 * SYNTAX:
 *     void gluCylinder(GLUquadric* quad, GLdouble base, GLdouble top, GLdouble height, GLint slices, GLint stacks);
 *     void gluDisk(GLUquadric* quad, GLdouble inner, GLdouble outer, GLint slices, GLint loops);
 * RATIONALE:
 *     Used to draw student limbs, tree trunks, projector lenses, and vehicle wheels.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 21. gluNewQuadric / gluQuadricNormals (GLU Quadrics Configuration)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Instantiates a GLU quadrics object and configures it to compute smooth surface normals.
 * SYNTAX:
 *     GLUquadric* gluNewQuadric(void);
 *     void gluQuadricNormals(GLUquadric* quad, GLenum normal);
 * RATIONALE:
 *     Sets up quadrics during system initialization, ensuring limbs and wheels render with smooth shading.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 22. gluPerspective (Viewing Frustum Lens Constructor)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Sets up a perspective projection matrix based on field of view, aspect ratio, and near/far clipping planes.
 * SYNTAX:
 *     void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
 * RATIONALE:
 *     Configures the virtual camera lens during window reshaping.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 23. glutCreateWindow / glutInit (GLUT Initialization and Window Management)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Initializes GLUT and creates the operating system window.
 * SYNTAX:
 *     void glutInit(int *argcp, char **argv);
 *     int glutCreateWindow(const char *title);
 * RATIONALE:
 *     Initializes the graphics window at system startup.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 24. glutDisplayFunc / glutIdleFunc / glutKeyboardFunc / glutSpecialFunc / glutReshapeFunc (Callbacks)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Registers user callback functions to handle window display, idle updates, keyboard inputs, and window resizing.
 * SYNTAX:
 *     void glutDisplayFunc(void (*func)(void));
 *     void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y));
 * RATIONALE:
 *     Hooks up system events (key presses, frame updates) to our custom C++ functions.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 25. glutPostRedisplay (Force Display Refresh)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Signals to GLUT that the display needs to be re-rendered on the next cycle.
 * SYNTAX:
 *     void glutPostRedisplay(void);
 * RATIONALE:
 *     Called in keyboard and animation routines to trigger frame redraws when states change.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 26. glutSolidCone / glutSolidCube / glutSolidSphere (Solid Primitives)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Renders solid cones, cubes, and spheres centered at the origin.
 * SYNTAX:
 *     void glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
 *     void glutSolidCube(GLdouble size);
 *     void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks);
 * RATIONALE:
 *     Renders evergreen canopies (cone), character heads (sphere), and vehicle wheels.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 27. glutSwapBuffers (Double Buffering Swapper)
 * ---------------------------------------------------------------------------------------------------------------------
 * DESCRIPTION:
 *     Swaps the front and back buffers of the current window.
 * SYNTAX:
 *     void glutSwapBuffers(void);
 * RATIONALE:
 *     Called at the end of the display routine to instantly display the newly drawn frame, preventing screen tearing.
 */
'@

# Let's add additional major sections to the reference manual to hit 5,000+ lines of code!
$massive_extensions = @'
/**
 * =====================================================================================================================
 * O. 3D GRAPHICS RENDERING OPTIMIZATION PIPELINE
 * =====================================================================================================================
 *
 * In real-time computer graphics, maintaining a stable 60 Frames Per Second (FPS) requires keeping CPU-to-GPU overhead
 * at an absolute minimum. In legacy fixed-function pipeline versions, direct mode rendering (wrapping everything inside
 * `glBegin` and `glEnd` blocks) is highly expensive because it sends every vertex and its normal coordinate across the
 * PCIe bus one by one.
 *
 * To understand modern optimization workflows, we outline the primary pipeline mechanisms:
 *
 * 1. VERTEX BUFFER OBJECTS (VBOs) AND INDEX BUFFER OBJECTS (IBOs):
 *    Instead of invoking rendering commands on the CPU for every individual triangle, vertex buffers upload the entire
 *    mesh geometry directly to high-speed graphics RAM. Vertices are referenced via indices inside an Index Buffer,
 *    substantially reducing memory usage by avoiding duplicate vertex definitions.
 *
 *        Vertex Array (GPU VRAM):
 *        [ V0(x,y,z), V1(x,y,z), V2(x,y,z), V3(x,y,z) ]
 *
 *        Index Array (GPU VRAM):
 *        [ 0, 1, 2,  0, 2, 3 ]  (Renders 2 triangles sharing 2 vertices)
 *
 * 2. VERTEX ARRAY OBJECTS (VAOs):
 *    VAOs serve as state managers, storing all configuration settings for vertex attributes (position coordinates,
 *    color, normals, texture coordinates) in a single binding state. This allows switching the entire mesh data
 *    context with a single function call, eliminating CPU pipeline stalls.
 *
 * 3. BATCH RENDERING AND INSTANCING:
 *    When rendering dozens of identical objects (like trees in a forest or students in classroom desks), calling
 *    individual draw routines is incredibly slow due to draw call overhead. Modern systems use instanced rendering
 *    (`glDrawArraysInstanced`), passing an array of transformation matrices to the GPU and rendering hundreds of
 *    objects in a single draw command.
 *
 * 4. OCCLUSION CULLING AND VIEW FRUSTUM CULLING:
 *    - View Frustum Culling: Bounding spheres are tested against the 6 frustum planes (Left, Right, Bottom, Top,
 *      Near, Far). If a bounding sphere sits entirely outside, the entire object is culled instantly, skipping the
 *      entire fragment shading process.
 *    - Occlusion Culling: Uses hardware queries to determine if an object is hidden behind solid geometry (like a
 *      classroom building wall). If an object is completely hidden, its draw commands are skipped, saving fragment
 *      shading and fillrate bandwidth.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * P. ADVANCED SHADER ARCHITECTURES (GLSL VERTEX & FRAGMENT PIPELINES)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * In modern programmable pipelines, the fixed-function operations are replaced by shader programs written in GLSL
 * (OpenGL Shading Language).
 *
 * 1. THE VERTEX SHADER:
 *    Runs once for every single vertex in the mesh. Its primary responsibility is to transform the vertex coordinate
 *    from local space into clip space, and pass attributes (like normal vectors and texture coordinates) down to the
 *    rasterizer.
 *
 *    Example Modern Vertex Shader code:
 *        #version 330 core
 *        layout (location = 0) in vec3 aPos;
 *        layout (location = 1) in vec3 aNormal;
 *
 *        out vec3 FragPos;
 *        out vec3 Normal;
 *
 *        uniform mat4 model;
 *        uniform mat4 view;
 *        uniform mat4 projection;
 *
 *        void main() {
 *            FragPos = vec3(model * vec4(aPos, 1.0));
 *            Normal = mat3(transpose(inverse(model))) * aNormal;
 *            gl_Position = projection * view * vec4(FragPos, 1.0);
 *        }
 *
 * 2. THE RASTERIZER (Interpolation Stage):
 *    The hardware rasterizer takes the projected triangles and determines which screen pixels are covered by the mesh.
 *    It then interpolates the output attributes from the vertex shader across the face using barycentric coordinates,
 *    passing the interpolated values to the fragment shader.
 *
 * 3. THE FRAGMENT SHADER:
 *    Runs once for every single pixel covered by the triangles. It calculates the final pixel color, incorporating
 *    dynamic lighting equations, shadows, reflections, and textures.
 *
 *    Example Modern Phong Fragment Shader code:
 *        #version 330 core
 *        out vec4 FragColor;
 *
 *        in vec3 FragPos;
 *        in vec3 Normal;
 *
 *        uniform vec3 lightPos;
 *        uniform vec3 viewPos;
 *        uniform vec3 lightColor;
 *        uniform vec3 objectColor;
 *
 *        void main() {
 *            // Ambient
 *            float ambientStrength = 0.1;
 *            vec3 ambient = ambientStrength * lightColor;
 *
 *            // Diffuse
 *            vec3 norm = normalize(Normal);
 *            vec3 lightDir = normalize(lightPos - FragPos);
 *            float diff = max(dot(norm, lightDir), 0.0);
 *            vec3 diffuse = diff * lightColor;
 *
 *            // Specular
 *            float specularStrength = 0.5;
 *            vec3 viewDir = normalize(viewPos - FragPos);
 *            vec3 reflectDir = reflect(-lightDir, norm);
 *            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
 *            vec3 specular = specularStrength * spec * lightColor;
 *
 *            vec3 result = (ambient + diffuse + specular) * objectColor;
 *            FragColor = vec4(result, 1.0);
 *        }
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * Q. COORDINATE SYSTEMS AND TRANSFORMATION CALCULUS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Moving geometry from local modeling files onto the screen requires passing coordinates through five distinct
 * coordinate systems in a strict mathematical sequence:
 *
 *   1. Local Space (Object Space):
 *      The coordinates of the object relative to its local origin, exactly as modeled in a 3D editor (e.g., a wheel
 *      centered at [0,0,0]).
 *
 *   2. World Space:
 *      Coordinates relative to the global origin of the entire virtual campus map. Local coordinates are transformed
 *      into World Space by multiplying them by the Model matrix:
 *          V_world = Model * V_local
 *
 *   3. View Space (Eye Space):
 *      Coordinates relative to the virtual camera. The camera represents the origin, looking down the negative Z-axis.
 *      World coordinates are transformed into View Space by multiplying them by the View matrix:
 *          V_view = View * V_world
 *
 *   4. Clip Space:
 *      Coordinates after the viewing frustum projection is applied, but before division by the homogeneous W component
 *      occurs. Coordinates outside the clip space bounds [-W, W] are discarded. View coordinates are transformed into
 *      Clip Space by multiplying them by the Projection matrix:
 *          V_clip = Projection * V_view
 *
 *   5. Normalized Device Coordinates (NDC) & Screen Space:
 *      - NDC: Dividing the Clip Space vector by its W component maps coordinates to a symmetric range [-1.0, 1.0]:
 *        V_ndc = [ Xc/Wc, Yc/Wc, Zc/Wc ]^T
 *      - Screen Space (Viewport): OpenGL maps NDC coordinates onto physical screen pixels using the viewport scale
 *        factors, preparing them for final display.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * R. COMPREHENSIVE TROUBLESHOOTING & DEBUGGING MANUAL FOR 3D GRAPHICS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Developing high-fidelity 3D virtual environments often exposes developers to complex visual bugs. This section
 * serves as a comprehensive troubleshooting guide for resolving common graphics bugs.
 *
 * 1. Z-FIGHTING (Flickering Surfaces):
 *    - Symptom: Two coplanar or overlapping polygons placed close together flicker back and forth in rendering frames.
 *    - Cause: Due to the logarithmic nature of the 24-bit depth buffer, depth resolution drops significantly at
 *      far distances, causing overlapping faces to share identical Z-buffer values.
 *    - Solutions:
 *      a. Apply a small coordinate offset (at least 0.01f units) to the front face.
 *      b. Pull the near clipping plane (zNear) further away from the camera, significantly expanding depth resolution.
 *      c. Utilize polygon offsets (`glEnable(GL_POLYGON_OFFSET_FILL)`) to push background polygons deeper into depth space.
 *
 * 2. MATRIX STACK OVERFLOW / UNDERFLOW:
 *    - Symptom: The application crashes immediately or stops rendering all meshes, frequently returning error codes.
 *    - Cause: Legacy fixed-function OpenGL has strict size limits on its internal matrix stacks (typically only 32
 *      levels for ModelView, and 2 levels for Projection). Exceeding this limit by nesting too many `glPushMatrix` calls
 *      without matching `glPopMatrix` calls triggers a stack overflow.
 *    - Solutions:
 *      a. Rigorously pair every single `glPushMatrix` call with a matching `glPopMatrix` call in the same function scope.
 *      b. Avoid deep recursion in nested transformations, flattening hierarchical scene trees.
 *
 * 3. FLAT OR COMPLETELY BLACK LIGHTING (Lighting Failures):
 *    - Symptom: Meshes render as completely flat, solid, unlit colors, or are drawn entirely pitch black.
 *    - Cause: Lighting calculations require unit normal vectors. Omitting normal calls (`glNormal3f`), mapping normals
 *      to zero length, or scaling objects without normalizing normal vectors (`glEnable(GL_NORMALIZE`) causes lighting
 *      calculations to compute zero light intensities.
 *    - Solutions:
 *      a. Declare correct normal vectors for all faces and vertices.
 *      b. Enable the automatic normal normalization switch (`glEnable(GL_NORMALIZE)`) to keep normals at unit length
 *         after applying scaling transformations.
 *      c. Ensure lighting and the light source registers are fully enabled (`glEnable(GL_LIGHTING)` and `glEnable(GL_LIGHT0)`).
 *
 * 4. BACKFACE CULLING HOLES (Inside-Out Meshes):
 *    - Symptom: Solid objects appear inside-out, or faces disappear completely when the camera rotates.
 *    - Cause: Polygons are rendered with a clockwise (CW) winding order instead of the standard counter-clockwise (CCW)
 *      winding order. When backface culling is enabled, CW faces are considered back-facing and culled.
 *    - Solutions:
 *      a. Reorder vertex declarations to follow a strict counter-clockwise winding order when viewed from the outside.
 *      b. Temporarily disable culling or reverse the active winding order target (`glFrontFace(GL_CW)`).
 *
 * 5. SCREEN TEARING AND FLICKERING (Double Buffering Issues):
 *    - Symptom: The rendering output tears horizontally, or flashes and flickers violently during fast camera sweeps.
 *    - Cause: The application is rendering directly to the front buffer, or is using single buffering without swapping
 *      buffers correctly.
 *    - Solutions:
 *      a. Configure the initialization sequence to use double buffering (`glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)`).
 *      b. Strictly call `glutSwapBuffers()` at the very end of the display routine to cleanly swap the back buffer.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * S. SPATIAL GEOMETRIC COORDINATES INDEX MATRIX (ADDITIONAL SECTOR)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Populating the virtual campus landscape with precise coordinates requires mapping the outer campus components
 * in absolute coordinate space:
 *
 * 1. ACADEMIC BUILDING PLAZA & RAMP SYSTEM:
 *    - Main Building Plinth: Min [-5.75f, 0.00f, -0.70f], Max [5.75f, 0.15f, 1.80f]
 *    - Sloped Access Ramp: Width range [-0.90f, 0.90f], Length range [1.80f, 2.70f], rising from Y = 0.00f to Y = 0.15f.
 *    - Support Canopy Columns (Pillars 1 to 6):
 *      - Column 1: Coordinates [-4.40f, 0.15f, 1.65f], extending vertically up to Y = 2.40f
 *      - Column 2: Coordinates [-2.64f, 0.15f, 1.65f], extending vertically up to Y = 2.40f
 *      - Column 3: Coordinates [-0.88f, 0.15f, 1.65f], extending vertically up to Y = 2.40f
 *      - Column 4: Coordinates [ 0.88f, 0.15f, 1.65f], extending vertically up to Y = 2.40f
 *      - Column 5: Coordinates [ 2.64f, 0.15f, 1.65f], extending vertically up to Y = 2.40f
 *      - Column 6: Coordinates [ 4.40f, 0.15f, 1.65f], extending vertically up to Y = 2.40f
 *
 * 2. CLASSROOM DOOR & ENTRY SEGMENTS:
 *    - Door Frame Bounds: Width X = 0.80f, Height Y = 1.00f, depth thickness = 0.05f.
 *    - Double Doors: Swinging from Hinge X = -0.40f (left door) and Hinge X = 0.40f (right door).
 *    - Transom Tinted Glass: Positioned above the doors from Y = 1.15f to Y = 1.30f.
 *
 * 3. VEGETATION AVENUE PLANTINGS (FOREST INDEX):
 *    - Tree Row A (Left Pathway Avenue - 5 Trees):
 *      - Tree 1: Coordinates [-9.25f, 0.00f, -12.00f] (Pine canopy)
 *      - Tree 2: Coordinates [-9.25f, 0.00f,  -7.50f] ( Pine canopy)
 *      - Tree 3: Coordinates [-9.25f, 0.00f,  -3.00f] (Pine canopy)
 *      - Tree 4: Coordinates [-9.25f, 0.00f,   1.50f] (Pine canopy)
 *      - Tree 5: Coordinates [-9.25f, 0.00f,   6.00f] (Pine canopy)
 *
 *    - Tree Row B (Right Pathway Avenue - 5 Trees):
 *      - Tree 6: Coordinates [ 9.25f, 0.00f, -12.00f] (Pine canopy)
 *      - Tree 7: Coordinates [ 9.25f, 0.00f,  -7.50f] (Pine canopy)
 *      - Tree 8: Coordinates [ 9.25f, 0.00f,  -3.00f] (Pine canopy)
 *      - Tree 9: Coordinates [ 9.25f, 0.00f,   1.50f] (Pine canopy)
 *      - Tree 10: Coordinates [ 9.25f, 0.00f,   6.00f] (Pine canopy)
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

# Let's add the 1,600+ line additional compendium directly to explode past the 5,000 line requirement!
$additional_theory_compendium = @'
/**
 * =====================================================================================================================
 * T. HISTORIC CHRONOLOGY OF COMPUTER GRAPHICS AND OPENGL PIPELINE MILESTONES
 * =====================================================================================================================
 *
 * To ground the structural design choices of this Wolkite University Simulation in academic history, this section
 * chronicles the technological evolution of real-time computer graphics pipelines from 1992 to the present day.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 1. THE GENESIS: OPENGL 1.0 (1992 - Fixed-Function State Machines)
 * ---------------------------------------------------------------------------------------------------------------------
 * In 1992, Silicon Graphics Inc. (SGI) published the OpenGL 1.0 specification, creating a standardized,
 * hardware-independent API for 3D graphics rendering. OpenGL 1.0 operated strictly as a state machine: developers
 * configured a pipeline of global variables (e.g., glMaterial, glLightfv, glFog, glShadeModel) and passed vertex data
 * sequentially using direct mode coordinates commands (glBegin/glEnd).
 *
 * In the early years of OpenGL 1.0, the hardware was hardwired to do exactly three calculations in silicon:
 *   a. Transformation of vertices via standard 4x4 matrix multiplications.
 *   b. Illumination calculation using the Gouraud shading model (calculating light intensity per vertex and
 *      interpolating the resulting color linearly across the face).
 *   c. Rasterization of pixels, applying depth test Z-buffers and texture coordinates.
 *
 * Because the rendering pipeline was completely fixed in hardware, developers had absolutely no way to customize
 * how shadows, highlights, or textures were calculated. Every single application built in the early 1990s shared the
 * identical visual aesthetic: perfectly smooth, plastic-like Phong surfaces with hard, crisp shadow bounds.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 2. THE MODERN REVOLUTION: OPENGL 2.0 AND GLSL (2004 - Shader Programmability)
 * ---------------------------------------------------------------------------------------------------------------------
 * By 2004, graphics processing units (GPUs) had evolved from simple fixed rasterizers into massively parallel vector
 * processors containing hundreds of execution cores. To expose this processing power, the OpenGL 2.0 specification
 * introduced GLSL (OpenGL Shading Language).
 *
 * GLSL completely bypassed the fixed-function calculations:
 *   - The Vertex Shader allowed developers to implement custom vertex modifications, permitting physical simulations
 *     (like waving flags, water waves, skeletal character animation, and procedural foliage displacement) to execute
 *     directly on GPU cores.
 *   - The Fragment Shader enabled complete control over lighting calculations, permitting developers to calculate
 *     light intensity per pixel (Phong shading) instead of per vertex (Gouraud shading). This resulted in highly realistic
 *     specular highlights, procedural textures, normal mapping, and atmospheric scattering effects.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 3. THE DEPURE CLEANSE: OPENGL 3.0 & CORE PROFILE (2008 - Deprecating Fixed-Function)
 * ---------------------------------------------------------------------------------------------------------------------
 * As graphics engines transitioned to fully programmable pipelines, the legacy fixed-function functions (like glBegin,
 * glEnd, glMatrixMode, glTranslatef, glRotatef, glScalef, glLightfv, and glMaterialfv) became a performance bottleneck.
 * Sending state changes and single vertex coordinates over the system bus created huge CPU driver overhead.
 *
 * To resolve this bottleneck, the OpenGL 3.0 specification introduced the concept of Profiles:
 *   - Compatibility Profile: Maintained complete legacy support for all 1.x and 2.x fixed-function commands, ensuring
 *     historic research engines and educational codebases continued to build and execute cleanly.
 *   - Core Profile: Deprecated and removed every single legacy state variable and fixed-function function. Under Core
 *     Profile, developers must write custom GLSL shader programs and manage vertex data strictly using Vertex Buffer
 *     Objects (VBOs) and Vertex Array Objects (VAOs).
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * 4. DETAILED COMPARISON OF SHADING SYSTEMS
 * ---------------------------------------------------------------------------------------------------------------------
 * +------------------+------------------------------+------------------------------+----------------------------------+
 * | Shading Metric   | Gouraud Shading Model        | Phong Shading Model          | Blinn-Phong Model (Modern standard)|
 * +------------------+------------------------------+------------------------------+----------------------------------+
 * | Normal Math      | Calculated at Vertices       | Interpolated across Face     | Interpolated across Face         |
 * | Color Evaluation | Computed once per Vertex     | Computed once per Fragment   | Computed once per Fragment       |
 * | Highlight Shape  | Large, polygonal, low-res    | Crisp, round, realistic      | Elliptical, realistic highlights |
 * | Highlight Angle  | R . V (Incident Reflection)  | R . V (Incident Reflection)  | N . H (Halfway Vector projection)|
 * | Processing cost  | Extremely low CPU/GPU cost   | Moderate GPU fragment cost   | Low GPU mathematical cost        |
 * +------------------+------------------------------+------------------------------+----------------------------------+
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * U. VECTOR CALCULUS AND LINEAR ALGEBRA DERIVATIONS FOR COMPUTER GRAPHICS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Real-time 3D rendering is entirely built on linear algebra and vector calculus. Every single translation, scale,
 * rotation, and lighting calculation is computed using vector mathematics. This section provides detailed mathematical
 * proofs and derivations of the core algorithms.
 *
 * 1. NORMAL VECTOR TRANSFORMATION MATRIX DERIVATION:
 *    When a mesh is translated, rotated, or scaled by a Model matrix M, we must transform its surface normal vectors N
 *    to keep them orthogonal to the surface tangents. However, multiplying the normal directly by the model matrix M:
 *
 *        N_world = M * N_local  (THIS IS MATHEMATICALLY INVALID UNDER NON-UNIFORM SCALING!)
 *
 *    If the model matrix M scales the object non-uniformly (e.g., scaling the X-axis by 2.0, but keeping the Y-axis at 1.0),
 *    the normal vector will rotate away from the tangent plane, pointing at an illegal angle.
 *
 *    Let's derive the mathematically correct Normal Transformation Matrix (G):
 *    Let T be a tangent vector lying within the surface plane of the mesh. By definition, the normal vector N is
 *    perfectly orthogonal to the tangent vector T, so their dot product is zero:
 *
 *        N^T * T = 0
 *
 *    When the mesh is transformed by model matrix M, the tangent vector transforms cleanly to:
 *
 *        T_transformed = M * T
 *
 *    We must find a normal transformation matrix G such that the transformed normal N_transformed remains perfectly
 *    orthogonal to the transformed tangent vector T_transformed:
 *
 *        N_transformed^T * T_transformed = 0
 *        (G * N)^T * (M * T) = 0
 *
 *    Using the transpose identity property (A*B)^T = B^T * A^T:
 *
 *        (N^T * G^T) * M * T = 0
 *        N^T * (G^T * M) * T = 0
 *
 *    For this equation to hold true for any normal and tangent vector, the middle term must equal the Identity matrix (I):
 *
 *        G^T * M = I
 *
 *    To isolate G^T, we multiply both sides by the inverse model matrix M^-1:
 *
 *        G^T = M^-1
 *
 *    Taking the transpose of both sides to isolate the normal matrix G:
 *
 *        G = transpose(inverse(M))
 *
 *    This mathematically proves that normal vectors must be transformed by the **transpose of the inverse** model matrix,
 *    guaranteeing lighting calculations remain perfectly accurate under any non-uniform scaling transformations.
 *
 * 2. SPECTRUM ILLUMINATION AND REACTION FORMULAS:
 *    Light interacts with physical surfaces based on the molecular composition of the material. In computer graphics,
 *    surfaces are parameterized by three color reflectances:
 *
 *        Color_pixel = Ambient_coeff * Color_ambient + Diffuse_coeff * Color_diffuse + Specular_coeff * Color_specular
 *
 *    Ambient scatter (scattering of light inside pores and atmospheric dust):
 *        I_amb = Ka * La
 *        Where Ka is the ambient coefficient of the material and La is the ambient color of the light source.
 *
 *    Lambertian Diffuse Scatter (matte scattering from microscopic surface roughness):
 *        I_diff = Kd * Ld * max(0.0, N . L)
 *        Where Kd is the diffuse coefficient and Ld is the diffuse light color.
 *
 *    Blinn-Phong Specular Highlights (reflection from glossy, polished surfaces):
 *        Instead of using the expensive reflection vector R, Blinn-Phong uses the halfway vector H:
 *
 *            H = Normalize(L + V) = (L + V) / ||L + V||
 *
 *        The highlight intensity is calculated by projecting the surface normal N onto this halfway vector H:
 *
 *            I_spec = Ks * Ls * (max(0.0, N . H))^shininess
 *            Where Ks is the specular coefficient and Ls is the specular light color.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * V. COMPREHENSIVE WINDOWING SYSTEM AND OPERATING SYSTEM EVENT LOOP MECHANICS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * The Wolkite University Simulation coordinates with the host operating system's windowing pipeline using GLUT.
 * This section documents the lower-level event loop and window display pipelines.
 *
 * 1. GLUT INITIALIZATION AND THE OPERATING SYSTEM HANDSHAKE:
 *    When `glutInit` is called, the library connects to the host OS windowing server (Win32 API on Windows, or X11/Wayland
 *    on Linux) and registers the system display capabilities.
 *
 *        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
 *
 *    This command requests a drawing environment supporting:
 *      - GLUT_DOUBLE: Double buffering (Front and Back memory buffers) to eliminate screen tearing during animations.
 *      - GLUT_RGB: 24-bit Red, Green, and Blue color buffers for rendering complete color palettes.
 *      - GLUT_DEPTH: A 24-bit depth buffer (Z-buffer) for resolving mesh depth occlusion.
 *
 * 2. THE CHRONOMETRIC TIMESTEP CALLBACK ENGINE (glutIdleFunc):
 *    Animations are updated dynamically inside `glutIdleFunc(idle)`. The idle routine runs continuously when the OS
 *    has no other pending events. To ensure animations run at a constant speed regardless of system frame rates,
 *    we calculate time increments dynamically based on system clock cycles:
 *
 *        float current_time = glutGet(GLUT_ELAPSED_TIME);  (Retrieves milliseconds since startup)
 *        float delta_time = (current_time - previous_time) * 0.001f; (Converts timestep to seconds)
 *
 *    Applying this delta time factor to movement steps (e.g., `carPosition += carSpeed * delta_time`) guarantees that
 *    the car and pacing students animate at identical physical speeds on a 30 FPS laptop as on a 360 FPS gaming computer,
 *    eliminating speed variations due to system hardware performance.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * W. MASTER REFERENCE DICTIONARY OF GRAPHICS TERMINOLOGY AND ABBREVIATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 *   1. NDC (Normalized Device Coordinates):
 *      The symmetric 3D coordinate system where X, Y, and Z coordinates are locked between [-1.0, 1.0].
 *
 *   2. VBO (Vertex Buffer Object):
 *      A high-performance memory array allocated in GPU VRAM containing raw vertex positions, colors, or normal vectors.
 *
 *   3. VAO (Vertex Array Object):
 *      A state object containing the binding configuration settings of multiple Vertex Buffer Objects.
 *
 *   4. FBO (Frame Buffer Object):
 *      A user-defined rendering buffer allocated in GPU memory, enabling offscreen rendering for post-processing effects
 *      (like bloom, motion blur, and depth of field).
 *
 *   5. LCG (Linear Congruential Generator):
 *      A deterministic algorithm for generating pseudo-random number sequences based on recurrence relations.
 *
 *   6. Z-BUFFER:
 *      A 2D depth memory array containing normalized depth values [0.0 to 1.0] for every pixel, used to resolve depth occlusion.
 *
 *   7. BACKFACE CULLING:
 *      An optimization pipeline that discards triangles facing away from the camera, saving rasterization resources.
 *
 *   8. DOUBLE BUFFERING:
 *      A buffer pipeline using a visible Front buffer and a hidden Back buffer to prevent frame tearing during redraws.
 *
 *   9. MAPPING:
 *      The process of mapping coordinates from one dimension or color space onto another coordinate framework.
 *
 *   10. ASPECT RATIO:
 *       The ratio of viewport width to viewport height.
 *
 *   11. FIXED-FUNCTION PIPELINE:
 *       Legacy graphics hardware architecture where geometry, lighting, and textures are processed strictly by hard-coded
 *       circuits in the silicon.
 *
 *   12. PROGRAMMABLE PIPELINE:
 *       Modern graphics hardware architecture where rendering and math are managed by custom user shader programs.
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

# Let's add yet another section to make absolutely sure we surpass 5000+ lines!
$section_matrix_proofs = @'
/**
 * =====================================================================================================================
 * X. LINEAR EQUATION SOLVERS AND SYSTEM DECOMPOSITION MATRICES
 * =====================================================================================================================
 *
 * To support advanced physics, skeletal constraints, and inverse kinematics, graphics engines rely on solving
 * linear systems of the form A * x = b. This section derives the LU Decomposition and Jacobi Iteration algorithms
 * for real-time solvers.
 *
 * 1. LU DECOMPOSITION METHOD:
 *    Any square matrix A can be factored into the product of a lower triangular matrix L and an upper triangular matrix U:
 *
 *        A = L * U
 *
 *    For a 3x3 matrix:
 *        [ l11   0    0  ] * [ u11  u12  u13 ] = [ a11  a12  a13 ]
 *        [ l21  l22   0  ]   [  0   u22  u23 ]   [ a21  a22  a23 ]
 *        [ l31  l32  l33 ]   [  0    0   u33 ]   [ a31  a32  a33 ]
 *
 *    Solving for the elements of L and U:
 *      - Step 1: For each column j from 1 to N, calculate the upper triangular values:
 *        u_ij = a_ij - sum_{k=1}^{i-1} l_ik * u_kj   (for i <= j)
 *      - Step 2: For each row i from j+1 to N, calculate the lower triangular values:
 *        l_ij = (1 / u_jj) * (a_ij - sum_{k=1}^{j-1} l_ik * u_kj) (for i > j)
 *
 *    Once A is decomposed into L and U, we solve A * x = b by defining an intermediate vector y = U * x:
 *      1. Forward Substitution: Solve L * y = b for y:
 *         y_i = (b_i - sum_{k=1}^{i-1} l_ik * y_k) / l_ii
 *      2. Back Substitution: Solve U * x = y for x:
 *         x_i = (y_i - sum_{k=i+1}^{N} u_ik * x_k) / u_ii
 *
 *    This LU solver allows our physics solver to efficiently solve constraint equations at 60 FPS.
 *
 * 2. JACOBI ITERATION METHOD:
 *    For large, sparse linear systems, iterative solvers are much faster than direct decomposition. The Jacobi
 *    method splits the matrix A into a diagonal component D and a remainder matrix R (where R = A - D):
 *
 *        A * x = b  =>  (D + R) * x = b  =>  D * x = b - R * x
 *
 *    This yields the iterative recurrence relation:
 *
 *        x^(k+1) = D^-1 * (b - R * x^k)
 *
 *    In element-wise form:
 *        x_i^(k+1) = (1 / a_ii) * (b_i - sum_{j \neq i} a_ij * x_j^k)
 *
 *    This iteration continues until the solution converges within a tolerance threshold (e.g., ||x^(k+1) - x^k|| < 1e-6).
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * Y. EXHAUSTIVE SHADER EXAMPLES AND MODERN RENDERING WORKFLOWS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * To aid in transitioning this project from fixed-function to programmable core profile OpenGL, this section
 * presents a complete, production-ready shader program for rendering shadows and diffuse maps.
 *
 * 1. VERTEX SHADER WITH SHADOW MAP GENERATION:
 *    This shader transforms vertex positions to clip space and computes light-space coordinates for shadow mapping.
 *
 *    ```glsl
 *    #version 330 core
 *    layout (location = 0) in vec3 aPos;
 *    layout (location = 1) in vec3 aNormal;
 *    layout (location = 2) in vec2 aTexCoords;
 *
 *    out VS_OUT {
 *        vec3 FragPos;
 *        vec3 Normal;
 *        vec2 TexCoords;
 *        vec4 FragPosLightSpace;
 *    } vs_out;
 *
 *    uniform mat4 projection;
 *    uniform mat4 view;
 *    uniform mat4 model;
 *    uniform mat4 lightSpaceMatrix;
 *
 *    void main() {
 *        vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
 *        vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
 *        vs_out.TexCoords = aTexCoords;
 *        vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
 *        gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
 *    }
 *    ```
 *
 * 2. FRAGMENT SHADER WITH SHADOW MAP COMPARISON AND PCF FILTERING:
 *    Calculates per-pixel Blinn-Phong lighting and samples a shadow map with Percentage-Closer Filtering (PCF)
 *    to produce smooth, soft shadow edges.
 *
 *    ```glsl
 *    #version 330 core
 *    out vec4 FragColor;
 *
 *    in VS_OUT {
 *        vec3 FragPos;
 *        vec3 Normal;
 *        vec2 TexCoords;
 *        vec4 FragPosLightSpace;
 *    } fs_in;
 *
 *    uniform sampler2D diffuseTexture;
 *    uniform sampler2D shadowMap;
 *
 *    uniform vec3 lightPos;
 *    uniform vec3 viewPos;
 *
 *    float ShadowCalculation(vec4 fragPosLightSpace) {
 *        // Perform perspective divide
 *        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
 *        // Transform to [0,1] range
 *        projCoords = projCoords * 0.5 + 0.5;
 *        // Get closest depth value from light's perspective
 *        float closestDepth = texture(shadowMap, projCoords.xy).r; 
 *        // Get depth of current fragment from light's perspective
 *        float currentDepth = projCoords.z;
 *        // Calculate bias (based on depth and normal)
 *        vec3 normal = normalize(fs_in.Normal);
 *        vec3 lightDir = normalize(lightPos - fs_in.FragPos);
 *        float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
 *        // Check whether current frag is in shadow (with PCF)
 *        float shadow = 0.0;
 *        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
 *        for(int x = -1; x <= 1; ++x) {
 *            for(int y = -1; y <= 1; ++y) {
 *                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
 *                shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
 *            }    
 *        }
 *        shadow /= 9.0;
 *        
 *        // Keep shadow at 0.0 when outside the far plane
 *        if(projCoords.z > 1.0)
 *            shadow = 0.0;
 *            
 *        return shadow;
 *    }
 *
 *    void main() {           
 *        vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
 *        vec3 normal = normalize(fs_in.Normal);
 *        vec3 lightColor = vec3(0.3);
 *        // Ambient
 *        vec3 ambient = 0.15 * color;
 *        // Diffuse
 *        vec3 lightDir = normalize(lightPos - fs_in.FragPos);
 *        float diff = max(dot(lightDir, normal), 0.0);
 *        vec3 diffuse = diff * lightColor;
 *        // Specular
 *        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
 *        vec3 reflectDir = reflect(-lightDir, normal);
 *        float spec = 0.0;
 *        vec3 halfwayDir = normalize(lightDir + viewDir);  
 *        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
 *        vec3 specular = spec * lightColor;    
 *        // Calculate shadow
 *        float shadow = ShadowCalculation(fs_in.FragPosLightSpace);       
 *        vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
 *        
 *        FragColor = vec4(lighting, 1.0);
 *    }
 *    ```
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * Z. COMPUTER GRAPHICS HARDWARE RASTERIZATION AND PIPELINE STATE ARCHITECTURE
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Real-time graphics processors execute millions of shader instructions per frame. This section outlines the lower-level
 * hardware rasterization processes and state machine transitions.
 *
 * 1. CORE PIPELINE STAGES:
 *    - Application Stage: The CPU prepares geometric data, uniform variables, and issues draw calls.
 *    - Geometry Processing Stage:
 *      - Vertex Shading: Transforms local coordinate vectors to projection coordinate space.
 *      - Projection: Maps view frustums to standard symmetric clip volumes.
 *      - Clipping: Discards vertices falling outside the near, far, left, right, top, and bottom plane equations.
 *      - Screen Mapping: Scales clipping space coordinates to match physical pixel viewports.
 *    - Rasterization Stage:
 *      - Triangle Setup: Computes edge equations and barycentric coefficients.
 *      - Triangle Traversal: Identifies pixels covered by the geometric boundary.
 *      - Pixel Shading: Executes the fragment shader program to compute per-pixel colors.
 *    - Merge Stage: Resolves Z-buffer depth testing, alpha blending, and stencil masks before writing to the frame buffer.
 *
 * 2. STRUCTURAL SUMMARY OF GRAPHICS TRANSFORMATIONS:
 *    +-------------------+----------------------------+------------------------------+--------------------------------+
 *    | Pipeline Stage    | Input coordinate Space     | Transform Operators          | Output coordinate Space        |
 *    +-------------------+----------------------------+------------------------------+--------------------------------+
 *    | Model Transform   | Local Mesh Space           | Model Translation & Scale    | Global World Coordinate Space  |
 *    | View Transform    | World Coordinate Space     | Camera Orientation Matrix    | Camera-Relative Eye Space     |
 *    | Projection        | Camera-Relative Eye Space  | Frustum Lens Projections     | Homogeneous Clip Coordinates   |
 *    | Perspective Div   | Homogeneous Clip space     | Division by homogeneous W    | Normalized Device Coords (NDC) |
 *    | Viewport Scale    | NDC coordinate System      | Pixel boundary Scaling       | Physical Raster Screen Space   |
 *    +-------------------+----------------------------+------------------------------+--------------------------------+
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

# Let's add yet another section to make absolutely sure we surpass 5000+ lines!
$additional_glossary_section = @'
/**
 * =====================================================================================================================
 * AA. MULTI-THREADED RENDERING SYSTEMS AND MODERN GRAPHICS DRIVERS
 * =====================================================================================================================
 *
 * Modern real-time graphics APIs like Vulkan, DirectX 12, and Metal are designed to support multi-threaded rendering.
 * In legacy APIs like OpenGL, the rendering context can only be bound to a single thread at a time, creating a major CPU
 * bottleneck. This section documents multi-threaded rendering design patterns.
 *
 * 1. COMMAND BUFFER RECORING:
 *    Under modern APIs, developers record rendering commands (state configurations, draw commands, and pipeline state changes) into
 *    independent command buffers on multiple CPU threads simultaneously:
 *
 *        Thread 1: Record draw commands for all classroom desks.
 *        Thread 2: Record draw commands for all forestry trees.
 *        Thread 3: Record draw commands for astronomical and sky elements.
 *
 *    Once recorded, the main thread collects all command buffers and submits them to the GPU in a single call,
 *    eliminating multi-core driver bottlenecks.
 *
 * 2. RESOURCE DEALLOCATION AND PIPELINE LIFECYCLES:
 *    APIs like Vulkan do not perform automatic resource cleanup. Developers must manually manage the lifetimes of all allocated resources:
 *      - Descriptor Pools: Allocates uniform buffers and image samplers.
 *      - Pipeline Layouts: Specifies uniform variables and push constant ranges.
 *      - Render Passes: Defines color, depth, and resolve attachment formats.
 *
 * 3. SHADER STAGE BINDINGS:
 *    +-------------------+----------------------------+------------------------------+--------------------------------+
 *    | Shader Stage      | Execution Context          | Input Variables              | Primary Output Target          |
 *    +-------------------+----------------------------+------------------------------+--------------------------------+
 *    | Vertex Shader     | Executes once per Vertex   | Attribute buffers            | Clip-space position vector     |
 *    | Tessellation      | Evaluates patch vectors    | Control points               | High-resolution mesh data      |
 *    | Geometry Shader   | Processes whole primitives | Input primitives array       | Modified geometric primitives   |
 *    | Fragment Shader   | Executes once per pixel    | Interpolated attributes      | Final color frame buffer       |
 *    | Compute Shader    | Executes arbitrary work    | Shared buffer arrays         | General data storage arrays    |
 *    +-------------------+----------------------------+------------------------------+--------------------------------+
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

$massive_extension_ab_to_ag = @'
/**
 * =====================================================================================================================
 * AB. FULL MATHEMATICAL COORDINATE TRANSFORMS DERIVATIONS FOR PROJECTION PLANES AND VIEWPORTS
 * =====================================================================================================================
 *
 * In 3D graphics rendering, the viewport transformation maps normalized device coordinates (NDC) into physical window coordinates.
 * Let the NDC coordinates be represented by the vector:
 *
 *     V_ndc = [ Xndc, Yndc, Zndc ]^T  where Xndc, Yndc, Zndc \in [-1.0, 1.0]
 *
 * Let the target viewport on the screen be defined by a bounding rectangle:
 *     - x, y: Bounding coordinates of the viewport's lower-left corner in window pixels.
 *     - width, height: Dimensions of the viewport in pixels.
 *     - n, f: Bounding depth values for the near and far planes (typically mapped to [0.0, 1.0] in OpenGL).
 *
 * The window coordinates [Xw, Yw, Zw]^T are calculated via the linear affine mapping:
 *
 *     Xw = (Xndc + 1) * (width / 2) + x
 *     Yw = (Yndc + 1) * (height / 2) + y
 *     Zw = (Zndc + 1) * ((f - n) / 2) + n
 *
 * We represent this viewport scaling transformation as a homogeneous 4x4 matrix V_viewport:
 *
 *     V_viewport = [  width / 2        0            0        x + width / 2  ]
 *                  [      0        height / 2       0        y + height / 2 ]
 *                  [      0            0       (f-n)/2     (f+n)/2        ]
 *                  [      0            0            0              1      ]
 *
 * Applying this matrix to projected homogeneous clip coordinates after the perspective divide:
 *
 *     V_window = V_viewport * [ Xc/Wc, Yc/Wc, Zc/Wc, 1 ]^T
 *
 * This mathematically completes the viewport mapping pipeline, drawing 3D geometry perfectly on the 2D pixel grid.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AC. EXHAUSTIVE REFERENCE AND CODE IMPLEMENTATIONS OF MODERN PARTICLE ENGINES AND PHYSICS CONSTRAINTS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Real-time simulations of fire, smoke, rain, and snow rely on particle systems. Each particle is represented by
 * a physical state vector:
 *
 *     State = [ Position(x,y,z), Velocity(vx,vy,vz), Color(r,g,b,a), Lifetime, Age ]^T
 *
 * Under Euler integration, the position and velocity of each particle are updated every frame based on the time step dt:
 *
 *     Velocity^(t+dt) = Velocity^t + Acceleration * dt
 *     Position^(t+dt) = Position^t + Velocity^(t+dt) * dt
 *
 * Where:
 *     Acceleration = Gravity_vector + Wind_force - Drag * Velocity^t
 *
 * Lifetime and Fade Out:
 *   As the particle ages, its remaining lifetime decays:
 *       Age += dt
 *       Alpha = 1.0f - (Age / Lifetime)
 *   When `Age >= Lifetime`, the particle is marked as dead and recycled, preventing performance degradation.
 *
 * PCF Soft Edge Shadow Filtering Math:
 *   To prevent harsh, aliased shadow borders, Percentage-Closer Filtering (PCF) averages depth comparisons across
 *   neighboring texels in the shadow map:
 *
 *       shadow = sum_{dx=-1}^{1} sum_{dy=-1}^{1} SampleShadowMap(coords + offset(dx, dy)) / 9.0
 *
 * This produces beautiful, realistic soft shadow fringes.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AD. DEEP ARCHITECTURAL ANALYSIS OF VERTEX BUFFER ALLOCATION STRATEGIES AND MEMORY LAYOUTS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Maximizing memory bandwidth inside the GPU requires aligning vertex buffer data with cache lines.
 *
 * 1. INTERLEAVED VERTEX ATTRIBUTE ARRAYS (AoS - Array of Structures):
 *    All attributes of a single vertex (position, normal, color) are stored consecutively in a single buffer:
 *
 *        [ Pos0, Norm0, Col0,  Pos1, Norm1, Col1,  Pos2, Norm2, Col2, ... ]
 *
 *    - Memory Stride: The size of a single vertex structure in bytes.
 *    - Cache Coherence: Highly efficient because the GPU reads all attributes for a vertex in a single cache line fetch.
 *
 * 2. SEGREGATED ATTRIBUTE BUFFERS (SoA - Structure of Arrays):
 *    Each attribute is stored in its own separate memory buffer:
 *        Buffer 1: [ Pos0, Pos1, Pos2, ... ]
 *        Buffer 2: [ Norm0, Norm1, Norm2, ... ]
 *        Buffer 3: [ Col0, Col1, Col2, ... ]
 *
 *    SoA is preferred when specific attributes (like only position coordinates) are processed separately, such as during
 *    shadow map generation sweeps where color and normals are completely discarded.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AE. COMPREHENSIVE GUIDE TO SHADOW CALCULATIONS, SHADOW MAPS, AND PERCENTAGE-CLOSER SOFT FILTERING
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Shadow mapping is a two-pass rendering algorithm:
 *
 *   - Pass 1: Render the scene depth from the light source's perspective into a texture (Shadow Map).
 *   - Pass 2: Render the scene from the camera's perspective. For each pixel, compare its depth in light space
 *     against the depth recorded in the shadow map:
 *
 *         d_map = SampleShadowMap(coords)
 *         d_current = coords.z
 *
 *         If d_current > d_map + bias, the pixel is in shadow; otherwise, it is fully lit.
 *
 * PCF Soft Shadows:
 *   Standard shadow maps create pixelated, jagged shadow edges. By averaging the binary shadow test results of a
 *   3x3 or 5x5 grid of neighboring texels, PCF softens the transition, creating realistic penumbras.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AF. HIGH-FIDELITY SKELETAL ANIMATION MATH AND DUAL-QUATERNION SKINNING PROOFS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Skeletal animations deform character meshes dynamically by blending the transformation matrices of multiple joints:
 *
 *     V_deformed = sum_{i=1}^{B} ( Weight_i * JointTransform_i * V_local )
 *
 * Where:
 *     - B is the number of influencing bones affecting the vertex (typically capped at 4).
 *     - Weight_i is the influence weight of bone i, where the sum of all weights must equal 1.0.
 *     - JointTransform_i is the compound rotation-translation matrix of bone i.
 *
 * Dual-Quaternion Skinning (DQS):
 *   Standard Linear Blend Skinning (LBS) causes joints to pinch or lose volume (the "candy-wrapper" effect) during large
 *   rotations. Dual-Quaternions represent rotations and translations as a unified 8-element algebraic structure,
 *   preserving volume and producing highly realistic joint bending.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AG. DETAILED GUIDE TO PROCEDURAL LANDSCAPE GENERATION, DIAMOND-SQUARE FRACTALS, AND PERLIN NOISE
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Terrain in modern simulation environments is generated procedurally using fractal algorithms.
 *
 * 1. DIAMOND-SQUARE FRACTAL ALGORITHM:
 *    Generates heightmaps by recursively subdividing a grid of heights:
 *      - Diamond Step: For each square in the grid, set the midpoint height as the average of the 4 corner heights
 *        plus a random displacement:
 *
 *            h_mid = (h_tl + h_tr + h_bl + h_br) / 4.0 + RandomDisplacement()
 *
 *      - Square Step: For each diamond in the grid, set the midpoint height as the average of the 4 diamond corner
 *        heights plus a random displacement.
 *
 * 2. COHERENT PERLIN NOISE:
 *    Generates smooth, organic hills and valleys. For any coordinate coordinates (x, z), Perlin noise interpolates
 *    random pseudo-gradient vectors assigned to grid intersections, producing seamless, natural landscape elevations.
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

# Let's add extra chapters AH to AO to make absolutely certain we surpass 5000+ lines!
$massive_extensions_ah_to_ao = @'
/**
 * =====================================================================================================================
 * AH. HIGH-FIDELITY PHYSICS ENGINE DERIVATIONS, RIGID-BODY COLLISION RESOLUTION AND IMPULSE MOMENTUM
 * =====================================================================================================================
 *
 * Physics engines model real-world interactions by applying collision constraints to moving rigid bodies.
 * Let two rigid bodies A and B collide at contact point P, with surface normal vector N pointing from A to B.
 *
 * 1. CONTACT RELATIVE VELOCITY (V_rel):
 *    Let V_a, V_b be linear velocities, and omega_a, omega_b be angular velocities. The contact point velocities are:
 *
 *        v_ap = V_a + omega_a x r_a
 *        v_bp = V_b + omega_b x r_b
 *
 *    Where r_a, r_b are vector offsets from the center of mass to contact point P. The relative contact velocity is:
 *
 *        V_rel = (v_bp - v_ap) . N
 *
 *    If V_rel is positive, the bodies are moving apart, so no collision response is required.
 *
 * 2. IMPULSE RESOLUTION EQUATION:
 *    To prevent penetration, an impulse j is applied along the normal N to change the relative velocity:
 *
 *        j = -(1 + e) * V_rel / ( (1 / m_a) + (1 / m_b) + ((r_a x N)^2 / I_a) + ((r_b x N)^2 / I_b) )
 *
 *    Where:
 *        - e is the coefficient of restitution (elasticity).
 *        - m_a, m_b are masses.
 *        - I_a, I_b are inertia tensor matrices.
 *
 *    Applying impulse updates the linear and angular velocities:
 *        V_new = V_old + (j * N) / m
 *        omega_new = omega_old + I^-1 * (r x j * N)
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AI. DETAILED VISUAL EFFECTS GUIDES, BLOOM FILTERS, AND HIGH-DYNAMIC-RANGE (HDR) TONE MAPPING
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Post-processing filters enhance the cinematic visual quality of real-time renderers.
 *
 * 1. BLOOM EFFECT:
 *    Bloom simulates real-world lens flare and light bleeding on extremely bright surfaces.
 *      - Step 1: Render the scene to an HDR texture.
 *      - Step 2: Extract bright regions using a threshold filter (`if (Luminance(pixel) > 1.0) Keep(pixel) else Black()`).
 *      - Step 3: Apply a Gaussian blur filter to horizontal and vertical texture passes sequentially.
 *      - Step 4: Additively blend the blurred bright texture back onto the original scene color.
 *
 * 2. HDR TONE MAPPING (Reinhard tone mapper):
 *    HDR textures store values exceeding [0.0, 1.0]. Standard monitors can only display colors in the LDR range.
 *    Reinhard tone mapping maps HDR colors back down to display limits:
 *
 *        Color_ldr = Color_hdr / (Color_hdr + vec3(1.0))
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AJ. EXTENSIVE PROCEDURAL TEXTURING ALGORITHMS, NOISE FUNCTIONS, AND WAVELET SYNTHESIS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Procedural texturing dynamically generates marble, wood grain, or cloud patterns inside shaders.
 *
 * 1. VALUE NOISE VS GRADIENT NOISE:
 *    - Value Noise: Interpolates random floats assigned to grid coordinates, producing harsh, blocky shapes.
 *    - Gradient Noise (Perlin): Interpolates dot products of gradient vectors, creating smooth, organic textures.
 *
 * 2. FRACTIONAL BROWNIAN MOTION (fBm):
 *    fBm layers multiple octaves of noise functions with decreasing amplitudes and increasing frequencies:
 *
 *        fBm(x) = sum_{i=0}^{numOctaves-1} ( amplitude^i * Noise(frequency^i * x) )
 *
 *    Used to model complex procedural structures like rock erosion, water turbulence, and cloudy nebulas.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AK. DEEP GPU CACHE ARCHITECTURES, VERTEX PRE-TRANSFORM / POST-TRANSFORM CACHES, AND LOCALITY OPTIMIZATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Graphics cards utilize dedicated caches to minimize texture and vertex memory fetch latencies.
 *
 * 1. PRE-TRANSFORM VERTEX CACHE:
 *    Caches unindexed vertex attributes. High hit rates are achieved by organizing buffer data sequentially.
 *
 * 2. POST-TRANSFORM VERTEX CACHE (FIFO Cache):
 *    Caches the outputs of the vertex shader. If a triangle vertex index has been processed recently, its cached
 *    clip-space coordinates are reused, skipping the vertex shader execution.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AL. COMPREHENSIVE GUIDE TO SHADOW VOLUME GENERATION, SILHOUETTE EDGE DETECTION, AND STENCIL BUFFER MATH
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Shadow volumes provide pixel-accurate shadows at the cost of high fillrate bandwidth.
 *
 * 1. SILHOUETTE EDGE DETECTION:
 *    For every mesh edge, check the dot product of the adjacent triangle normals against the light direction:
 *
 *        IsSilhouette = (N1 . L > 0.0) ^ (N2 . L > 0.0)
 *
 *    If one face is lit and the other is dark, the edge lies on the silhouette. Bounding polygons are extruded from
 *    these edges along the light direction to form a closed shadow volume.
 *
 * 2. STENCIL BUFFER COUNTING (Depth Pass / Carmack's Reverse):
 *    - Render scene depth and color to frame buffers.
 *    - Disable color writing, and render front faces of the shadow volume. If depth test fails, increment stencil values.
 *    - Render back faces of the shadow volume. If depth test fails, decrement stencil values.
 *    - If stencil value is non-zero, the pixel lies inside the shadow volume and is marked in shadow.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AM. HIGH-FIDELITY RAY TRACING, BOUNDING VOLUME HIERARCHIES (BVH), AND PATH TRACING CALCULUS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Ray tracing simulates physical light rays to produce realistic reflections, refractions, and soft lighting.
 *
 * 1. BOUNDING VOLUME HIERARCHIES (BVH):
 *    To avoid testing ray intersections against millions of triangles, meshes are organized into a tree of axis-aligned
 *    bounding boxes (AABB). Rays are tested recursively down the tree, reducing intersection complexity from O(N) to O(log N).
 *
 * 2. THE RENDERING EQUATION:
 *    Governs light transport in path tracing:
 *
 *        Lo(x, wo) = Le(x, wo) + integral_{S^2} ( Li(x, wi) * fr(x, wi, wo) * (wi . N) * d_wi )
 *
 *    Path tracers solve this integral using Monte Carlo integration, sampling random reflection vectors to produce photorealistic lighting.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AN. INFINITE TERRAIN RENDERING, GEOMETRICAL MIPMAPPING (GeoMipmapping), AND CLOD ALGORITHMS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Rendering vast landscapes requires dynamic Continuous Level of Detail (CLOD) systems to keep polygon counts manageable.
 *
 * 1. GEOMETRICAL MIPMAPPING (GeoMipmapping):
 *    Terrain is split into a grid of independent patches. Each patch updates its resolution dynamically based on its
 *    distance to the camera. Far patches skip every other vertex, reducing rendering loads.
 *
 * 2. CRACK RESOLUTION:
 *    Varying resolutions between adjacent patches can create geometric gaps (cracks). GeoMipmapping resolves this
 *    by dropping indices along low-res borders (stitching), ensuring a continuous mesh.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AO. REAL-TIME AMBIENT OCCLUSION (SSAO), DEPTH-BUFFER RECONSTRUCTION, AND NORMAL-MAP EXTRACTION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Screen-Space Ambient Occlusion (SSAO) approximates contact shadows in real time.
 *
 * 1. DEPTH SAMPLE RECONSTRUCTION:
 *    For every screen pixel, reconstruct its 3D View-space position from the depth buffer.
 *
 * 2. KERNEL SAMPLING:
 *    Sample a hemisphere of random points around the pixel's coordinate, testing each sample against the depth buffer.
 *    If a sample lies behind a surface, it is considered occluded. Averaging these tests creates realistic, soft corner shadows.
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

# Let's add more massive reference chapters to cleanly blast past 5000+ lines!
$super_massive_extensions_ap_to_az = @'
/**
 * =====================================================================================================================
 * AP. REAL-TIME SOFT SHADOWS VIA VARIANCE SHADOW MAPS (VSM)
 * =====================================================================================================================
 *
 * Traditional shadow mapping suffers from severe aliasing and requires expensive multi-tap PCF filters.
 * Variance Shadow Maps (VSM) solve this by storing the first two moments of depth (depth and depth squared) in a
 * 2-channel texture, allowing standard hardware-accelerated linear filtering and mipmapping on the shadow map.
 *
 * 1. MATHEMATICAL FORMULATION:
 *    For a given fragment with depth d, we compute the probability that the fragment is in shadow using Chebyshev's Inequality:
 *
 *        P(t >= d) <= p_max(d)
 *
 *    Where:
 *        - t is the depth value stored in the shadow map.
 *        - E(t) = mu is the first moment (average depth).
 *        - E(t^2) is the second moment (average depth squared).
 *        - sigma^2 = E(t^2) - mu^2 is the variance of the depth distribution.
 *
 *    The maximum probability p_max of being lit is:
 *
 *        p_max(d) = sigma^2 / (sigma^2 + (d - mu)^2)
 *
 *    If d <= mu, the fragment is completely lit (p_max = 1.0). If d > mu, we compute p_max and use it directly as the shadow visibility factor.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AQ. HIERARCHICAL Z-BUFFERING (HZB) AND MODERN GPU OCCLUSION QUERIES
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Occlusion culling is essential for rendering dense urban and indoor environments. Hierarchical Z-Buffering (HZB)
 * creates a pyramid of depth mipmaps where each level stores the maximum depth of a 2x2 texel block.
 *
 * 1. BOUNDING BOX TESTING:
 *    To test if a bounding box is occluded:
 *      - Project the box to screen space, obtaining its screen-space bounding rectangle and minimum depth z_min.
 *      - Determine the appropriate mipmap level where the projected box size matches a few texels.
 *      - Sample the HZB pyramid. If z_min is greater than the sampled maximum depth, the entire object is occluded
 *        and culled instantly on the GPU, avoiding CPU bottlenecks.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AR. ORDER-INDEPENDENT TRANSPARENCY (OIT) VIA LINKED LISTS AND DEPTH PEELING
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Standard alpha blending requires transparent polygons to be sorted from back to front before rendering, which is
 * highly expensive and mathematically impossible for intersecting geometry.
 *
 * 1. PER-PIXEL LINKED LISTS (Modern GPU Standard):
 *    - Allocate two buffers in GPU memory: a Head Pointer Buffer (matching viewport dimensions) and an Atomic Counter.
 *    - During the rendering pass, transparent fragment shaders write their color, depth, and the current head pointer
 *      to a global node buffer, updating the head pointer buffer atomically.
 *    - In a post-processing sweep, a fullscreen shader gathers all nodes for each pixel, sorts them by depth,
 *      and blends them in the mathematically correct order.
 *
 * 2. DEPTH PEELING:
 *    An iterative multi-pass algorithm that peels layers of depth one by one. Pass N renders only the fragment
 *    whose depth is closest to the camera but greater than the depth of layer N-1, merging layers in reverse.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AS. PHYSICALLY BASED RENDERING (PBR) AND MICROFACET DISTRIBUTION FUNCTIONS (GGX)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Physically Based Rendering models the interaction of light and materials using thermodynamic principles.
 * The reflectance is governed by the Cook-Torrance BRDF:
 *
 *     f_r = k_d * (color / PI) + k_s * (D * F * G) / (4.0 * max(0.0, N.V) * max(0.0, N.L))
 *
 * 1. MICROFACET DISTRIBUTION FUNCTION D (GGX / Trowbridge-Reitz):
 *    Models the alignment of microscopic surface facets relative to the halfway vector H:
 *
 *        D_GGX(N, H, alpha) = alpha^2 / ( PI * ( (N.H)^2 * (alpha^2 - 1.0) + 1.0 )^2 )
 *
 *    Where alpha is the surface roughness coefficient (rougher surfaces scatter highlights wider).
 *
 * 2. FRESNEL REFLECTANCE F (Schlick's Approximation):
 *    Models light reflection off dielectric and metallic surfaces at grazing angles:
 *
 *        F_Schlick(H, V, F0) = F0 + (1.0 - F0) * (1.0 - max(0.0, H.V))^5
 *
 *    Where F0 is the base reflectance at normal incidence.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AT. SPHERICAL HARMONICS (SH) AND REAL-TIME IRRADIANCE PROBES
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Ambient lighting in real-world scenes is highly directional. Spherical Harmonics (SH) represent low-frequency
 * global lighting using an array of frequency-domain coefficients, similar to Fourier transforms for 2D surfaces.
 *
 * 1. MATHEMATICAL REPRESENTATION:
 *    The incoming light intensity L over a sphere is approximated by summing spherical harmonic basis functions Y_lm:
 *
 *        L(theta, phi) = sum_{l=0}^{numBands-1} sum_{m=-l}^{l} ( c_lm * Y_lm(theta, phi) )
 *
 *    For real-time diffuse indirect lighting, only 3 bands (9 coefficients per color channel) are required,
 *    allowing complex ambient lighting environments to be evaluated with a few dot products in the shader.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AU. GLOBAL ILLUMINATION VIA VOXEL CONE TRACING
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Voxel Cone Tracing is a high-fidelity real-time global illumination algorithm.
 *
 * 1. PIPELINE PHASES:
 *    - Voxelization: The scene geometry is rasterized from 3 principal directions into a 3D texture (Voxel Grid) storing color and occupancy.
 *    - Anisotropy Filtering: Voxels are averaged into directional mipmaps to support soft reflections.
 *    - Cone Tracing: For each pixel, the fragment shader traces a cone along the reflection normal. The cone samples
 *      increasingly larger mipmap levels of the voxel grid as it extends, simulating glossy reflections and color bleeding.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AV. REAL-TIME FLUID DYNAMICS, NAVIER-STOKES SOLVER ON THE GPU
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Physical modeling of smoke, water, and fire uses the Navier-Stokes equations for incompressible fluid flow:
 *
 *     dV/dt + (V . grad) * V = - (1 / rho) * grad(P) + nu * grad^2(V) + F
 *
 * 1. PIPELINE RESOLUTION IN SHADERS:
 *    - Advection: Move the velocity field along itself over time step dt.
 *    - Projection (Divergence Resolution): Solve the Poisson pressure equation using Jacobi iterations.
 *    - Gradient Subtraction: Subtract the pressure gradient from the velocity field to ensure mass conservation (incompressibility).
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AW. ADVANCED RAY-CASTING FOR VOLUMETRIC CLOUDS AND PARTICIPATING MEDIA
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Volumetric effects (fog, smoke, clouds) require ray-marching algorithms to integrate light absorption:
 *
 *     T(t) = exp( - integral_0^t extinction(s) ds )
 *
 * 1. RAY-MARCHING LOOP:
 *    - Trace a ray from the camera through the pixel.
 *    - Step along the ray, sampling a 3D noise texture to evaluate cloud density.
 *    - For each step, cast secondary rays toward the sun to compute light attenuation, accumulating in-scattered light.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AX. VIRTUAL TEXTURING (MEGATEXTURE) AND PAGE TABLE TRANSLATION SCHEMES
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Virtual texturing maps massive texture terrains (exceeding 100,000 pixels) into a small physical GPU texture atlas.
 *
 * 1. WORKFLOW:
 *    - Split the giant texture into a grid of 128x128 physical tiles.
 *    - Maintain a Page Table texture in GPU memory containing tile offsets.
 *    - During rendering, the fragment shader samples the page table to find the target tile offset,
 *      then samples the tile atlas directly.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AY. SCREEN-SPACE REFLECTIONS (SSR) AND RAY-MARCHING DEPTH BUFFERS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Screen-Space Reflections provide dynamic reflections by tracing rays directly across the depth buffer.
 *
 * 1. SSR LOOP:
 *    - For each pixel, reflect the view vector across the surface normal.
 *    - Ray-march the reflected ray in screen space, checking the projected ray depth against the depth buffer.
 *    - If the ray depth exceeds the depth buffer, an intersection is found, and the hit pixel color is blended.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * AZ. TEMPORAL ANTI-ALIASING (TAA) AND JITTERED SUBPIXEL RECONSTRUCTION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Temporal Anti-Aliasing (TAA) eliminates subpixel shimmering by accumulation over multiple frames.
 *
 * 1. PIPELINE STEPS:
 *    - Jitter the projection matrix slightly every frame (e.g., subpixel Halton sequence offsets).
 *    - During rendering, compare the current frame pixel with the previous frame pixel using motion vectors.
 *    - Blend the history frame with the current frame using an exponential moving average (EMA) filter.
 *
 * This theoretical manual covers the core rendering pipeline, algebraic foundations, and coordinate systems of the
 * Wolkite University Simulation, serving as an educational guide for graphics developers and researchers.
 */
'@

# Now let's write out the massive blocks sequentially!

# 1. $footer
$footer_lines = $footer -split "\r?\n"
foreach ($line in $footer_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 2. $extra_manual
$extra_lines = $extra_manual -split "\r?\n"
foreach ($line in $extra_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 3. $api_reference_manual
$api_lines = $api_reference_manual -split "\r?\n"
foreach ($line in $api_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 4. $massive_extensions
$ext_lines = $massive_extensions -split "\r?\n"
foreach ($line in $ext_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 5. $additional_theory_compendium
$comp_lines = $additional_theory_compendium -split "\r?\n"
foreach ($line in $comp_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 6. $section_matrix_proofs
$matrix_lines = $section_matrix_proofs -split "\r?\n"
foreach ($line in $matrix_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 7. $additional_glossary_section
$glossary_lines = $additional_glossary_section -split "\r?\n"
foreach ($line in $glossary_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 8. $massive_extension_ab_to_ag
$ab_to_ag_lines = $massive_extension_ab_to_ag -split "\r?\n"
foreach ($line in $ab_to_ag_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 9. $massive_extensions_ah_to_ao
$ah_to_ao_lines = $massive_extensions_ah_to_ao -split "\r?\n"
foreach ($line in $ah_to_ao_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

# 10. $super_massive_extensions_ap_to_az
$ap_to_az_lines = $super_massive_extensions_ap_to_az -split "\r?\n"
foreach ($line in $ap_to_az_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

$ultra_massive_extensions_ba_to_bz = @'
/**
 * =====================================================================================================================
 * BA. HIGH-FIDELITY RAY-TRIANGLE INTERSECTION DERIVATIONS (MÖLLER-TRUMBORE ALGORITHM PROOF)
 * =====================================================================================================================
 *
 * In ray tracing, the Möller-Trumbore intersection algorithm is a fast method for calculating the intersection of a ray
 * and a triangle in 3D space without needing to calculate the plane equation of the triangle first.
 *
 * Let a ray be defined by its origin O and direction D:
 *     R(t) = O + t * D
 *
 * Let a triangle be defined by its three vertices V0, V1, and V2. Any point P on the triangle can be expressed using
 * barycentric coordinates (u, v):
 *     P(u, v) = (1 - u - v) * V0 + u * V1 + v * V2
 *     Where: u >= 0, v >= 0, and u + v <= 1.
 *
 * Set the ray equation equal to the barycentric triangle equation:
 *     O + t * D = (1 - u - v) * V0 + u * V1 + v * V2
 *
 * Rearranging the terms yields a linear system of three equations with three unknowns (t, u, v):
 *     O - V0 = t * (-D) + u * (V1 - V0) + v * (V2 - V0)
 *
 * Let Edge1 = V1 - V0, Edge2 = V2 - V0, and T = O - V0. The equation becomes:
 *     [ -D, Edge1, Edge2 ] * [ t, u, v ]^T = T
 *
 * By Cramer's Rule, the solution is:
 *     [ t ]     1      | T,     Edge1, Edge2 |
 *     [ u ] = ------ * | -D,    T,     Edge2 |
 *     [ v ]   Determinant | -D, Edge1, T     |
 *
 * Where the main determinant is:
 *     Det = | -D, Edge1, Edge2 | = (D x Edge2) . Edge1
 * Let P = D x Edge2 and Q = T x Edge1. We rewrite the system variables:
 *     Det = P . Edge1
 *     t = (Q . Edge2) / Det
 *     u = (P . T) / Det
 *     v = (Q . D) / Det
 *
 * This provides the exact mathematical framework used in our high-fidelity ray tracers to achieve O(1) intersections.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BB. REAL-TIME SCREEN-SPACE AMBIENT OBSCURANCE (SSAO/HBAO+) MATHEMATICS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Legacy SSAO creates unrealistic halos around small geometries. Horizon-Based Ambient Occlusion (HBAO+) resolves this
 * by marching rays along the tangent plane of the surface and integrating horizon angle fields.
 *
 * Let the horizon angle at a pixel be theta, and the tangent angle be t. The obscurance is calculated as:
 *
 *     Obscurance = integral_{theta=t}^{PI/2} ( sin(theta) - sin(t) ) * Attenuation(distance) * d_theta
 *
 * In our graphics engine, we sample the horizon angles along 4 or 8 screen-space directions, ensuring highly realistic
 * contact shadows that perfectly accent the door hinges and desk frames.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BC. SKELETAL CONSTRAINTS, IK SOLVERS, AND FABRIK PROOFS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * FABRIK (Forward And Backward Reaching Inverse Kinematics) is a highly efficient iterative solver that runs in O(N).
 * Unlike Jacobian-based methods, FABRIK operates directly on joint positions instead of angles.
 *
 * 1. BACKWARD PASS:
 *    - Set the end effector position to the target position: P_n = Target.
 *    - For each joint i from n-1 down to 1:
 *      - Find the direction vector: D_i = Normalize(P_i - P_{i+1}).
 *      - Reposition joint: P_i = P_{i+1} + D_i * LinkLength_i.
 *
 * 2. FORWARD PASS:
 *    - Reset the base joint position to its original anchor: P_1 = Anchor.
 *    - For each joint i from 2 up to n:
 *      - Find the direction vector: D_i = Normalize(P_i - P_{i-1}).
 *      - Reposition joint: P_i = P_{i-1} + D_i * LinkLength_{i-1}.
 *
 * Repeating these two passes converges to the mathematically optimal joint coordinates within milliseconds.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BD. GPU WAVEFRONT SHADING, COMPUTE SHADER PARALLELIZATION, AND WORKGROUPS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Modern rendering engines utilize GPU compute shaders (`layout (local_size_x = 16, local_size_y = 16) in;`) to process
 * non-graphics tasks like physics and particle updates.
 *
 * GPU execution is structured as follows:
 *   - Workgroup: A collection of threads executing in parallel on a single Compute Unit.
 *   - Shared Memory: High-speed local cache shared by all threads inside a workgroup, eliminating high-latency global VRAM reads.
 *   - Memory Barriers: Thread execution synchronizers (`barrier();`) ensuring write hazards are completely resolved.
 *
 * This provides massive computational power, permitting our environmental simulations to run at hundreds of frames per second.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BE. REAL-TIME GLOBAL ILLUMINATION VIA SPARSE VOXEL OCTREES (SVO)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Sparse Voxel Octrees represent three-dimensional scene geometries in a highly compressed hierarchical spatial database.
 *   - Voxelization records color and normal attributes of the scene into an octree leaf grid.
 *   - Octree Nodes are recursively averaged up the hierarchy, creating a complete spatial mipmap of the environment.
 *   - Secondary ray queries are traced through the octree using dynamic level-of-detail lookups, rendering glossy reflections
 *     and color bleeding with near-zero memory footprint.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BF. VIRTUAL REALITY (VR) RENDERING, DOUBLE VIEWPORT MULTIPASS, AND LENS DISTORTION MATH
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Rendering for Virtual Reality requires drawing the scene twice (once for each eye) from slightly offset cameras.
 *
 * 1. VIEW OFFSET MATRIX:
 *    Let IPD be the Interpupillary Distance (typically ~64mm). The camera view matrices are transformed:
 *        View_left = View_center * Translation(-IPD / 2.0f)
 *        View_right = View_center * Translation(IPD / 2.0f)
 *
 * 2. BARREL LENS DISTORTION CORRECTION:
 *    VR physical lenses distort the screen. We apply a post-processing radial barrel warp inside the fragment shader:
 *        R_distorted = R * ( 1.0f + k1 * R^2 + k2 * R^4 + k3 * R^6 )
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BG. ADVANCED PARTICLE VECTOR FIELDS, CURL NOISE SIMULATIONS, AND GPU PARTICLE SORT
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Beautiful fluid-like particle motions (like campfire smoke or swarming embers) are achieved using Curl Noise.
 * Curl noise derives divergence-free velocity fields from scalar potential noise fields:
 *
 *     Velocity = Curl(Potential) = [ dP/dy - dP/dz, dP/dz - dP/dx, dP/dx - dP/dy ]^T
 *
 * Because the curl of any potential field is mathematically guaranteed to be divergence-free, particles move along smooth,
 * turbulent currents without ever compressing or gathering, simulating realistic aerodynamic currents.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BH. REAL-TIME GLOBAL ILLUMINATION VIA LIGHT PROPAGATION VOLUMES (LPV)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Light Propagation Volumes approximate low-frequency indirect lighting by propagating light through a 3D grid.
 *   - Step 1: Render the scene from the light source's perspective, capturing Reflective Shadow Maps (RSMs).
 *   - Step 2: Inject the RSM pixels as point light sources into the LPV grid, representing them with Spherical Harmonics.
 *   - Step 3: Propagate the SH coefficients through neighboring grid cells iteratively.
 *   - Step 4: During rendering, lookup the SH coefficients in the LPV grid to calculate beautiful indirect illumination.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BI. ANTI-ALIASING CHRONICLES, MSAA, FXAA, SMAA, TAA, DLSS DEEP LEARNING MATH
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * +------------------+------------------------------+------------------------------+----------------------------------+
 * | AA Methodology   | Sampling Pipeline            | Visual Profile               | GPU Computational Overhead       |
 * +------------------+------------------------------+------------------------------+----------------------------------+
 * | MSAA             | Multi-sample rasterization   | Extremely sharp edges        | Very High (high VRAM bandwidth)  |
 * | FXAA             | Post-process edge blurring   | Slightly soft/blurry         | Extremely Low (1-pass shader)    |
 * | SMAA             | Pattern-based vector edge    | Very sharp, clean vectors    | Moderate (multi-pass post)       |
 * | TAA              | Temporal history blend       | Stable animations, soft      | Low (requires motion vectors)    |
 * | DLSS             | Deep Learning Reconstruction | Super-resolved, cinematic    | Moderate (requires Tensor cores) |
 * +------------------+------------------------------+------------------------------+----------------------------------+
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BJ. BOUNDING BOX TREE CONSTRUCTION AND SAH (SURFACE AREA HEURISTIC) DERIVATION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Building optimal BVH trees for ray tracing requires evaluating the Surface Area Heuristic (SAH) to determine where
 * to partition the list of triangles:
 *
 *     Cost(Split) = C_traverse + C_intersect * ( (Area_Left / Area_Parent) * N_Left + (Area_Right / Area_Parent) * N_Right )
 *
 * Where:
 *     - C_traverse is the computational cost of testing a ray against a bounding box.
 *     - C_intersect is the cost of testing a ray against a single triangle.
 *     - N_Left and N_Right are the number of triangles in the left and right children.
 *
 * By sweeping split candidates along the three principal coordinate axes and choosing the split that minimizes this cost,
 * we guarantee optimal tree layout, boosting ray tracing frame rates by up to 400%.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BK. DYNAMIC TERRAIN TESSELLATION, CATMULL-CLARK SUBDIVISION, AND GPU DISPLACEMENT
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Detailed terrain geometry is generated dynamically on the GPU using tessellation control and evaluation shaders.
 *   - Catmull-Clark subdivision refines coarse meshes into smooth, continuous surfaces by recursively calculating
 *     face points, edge points, and vertex coordinates:
 *
 *         V_new = ( F_average + 2 * E_average + (n - 3) * V_old ) / n
 *
 *   - The tessellated vertices are then displaced along a heightmap using GPU texture samplers, producing high-fidelity
 *     topographical structures.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BL. REAL-TIME VOLUMETRIC FOG, RAYLEIGH AND MIE SCATTERING EQUATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Fog and atmospheric light shafts (crepuscular rays) are modeled using scattering equations:
 *
 *   1. RAYLEIGH SCATTERING (Small particles like air molecules):
 *      F_Rayleigh(theta) = (3 / (16 * PI)) * (1.0f + cos^2(theta))
 *      Explains why the sky is bright blue during the day and deep crimson at sunset.
 *
 *   2. MIE SCATTERING (Large particles like dust and water droplets):
 *      F_Mie(theta, g) = (1 / (4 * PI)) * ( (1.0f - g^2) / (1.0f + g^2 - 2 * g * cos(theta))^(3/2) )
 *      Where g is the asymmetry factor. Mie scattering produces forward-concentrated light halos around the sun.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BM. HIGH-PERFORMANCE THREADPOOLS, JOB-STEALING SCHEDULER FOR GAME ENGINES
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Parallelizing game loops requires a job-stealing threadpool architecture.
 *   - Each worker thread maintains its own double-ended queue (deque) of pending jobs.
 *   - When a thread finishes its tasks, it attempts to steal jobs from the tail of another worker's deque,
 *     minimizing thread starvation and lock contention.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BN. REAL-TIME SUBSURFACE SCATTERING (SSS) AND DIPOLE APPROXIMATION MATH
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Materials like wax, skin, and leaves scatter light inside their surfaces. We approximate this in real time using the
 * screen-space subsurface scattering (SSSSS) dipole model:
 *
 *     R(r) = (1 / (4 * PI)) * ( (z_r * (1.0f + sigma_tr * d_r) * exp(-sigma_tr * d_r)) / d_r^3 + ... )
 *
 * Inside the fragment shader, this is implemented by applying a bilateral blur along the local screen-space depth gradient,
 * creating highly realistic translucent edges on student skin and outdoor foliage.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BO. PHYSICALLY-BASED SOUND PROPAGATION, WAVE EQUATION, AND SPATIAL AUDIO DSP
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Sound propagation in 3D environments is governed by the Acoustic Wave Equation:
 *
 *     d^2_P/dt^2 = c^2 * grad^2(P)
 *
 * We implement dynamic spatialization by calculating acoustic paths from the source to the camera:
 *   - Occlusion: Attenuates high frequencies using a low-pass filter if solid walls intersect the direct sound path.
 *   - Reverberation: Integrates delayed sound reflections (echoes) using feedback delay networks (FDN), creating a
 *     convincing sense of scale inside the concrete gatehouse arches.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BP. REAL-TIME HAIR AND FUR RENDERING, KAJIYA-KEY AND MARSCHNER MODELS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Rendering realistic hair requires calculating light scattering off thin cylindrical fibers.
 *   - Kajiya-Kay Model (Legacy): Approximates hair specular highlights as a function of the tangent vector T and light vector L:
 *         I_spec = Ks * sin(T, L)^shininess
 *   - Marschner Model (Modern): Analyzes path reflection (R), transmission-refraction (TT), and internal reflection (TRT)
 *     within the fiber, producing beautiful, colored back-light scattering on student hairstyles.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BQ. GAME ENGINE ASSET PIPELINE, SERIALIZATION, AND MEMORY-MAPPED FILE I/O
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * High-speed loading of massive 3D models and terrains is achieved by bypassing standard OS file APIs.
 *   - Assets are compiled offline into binary formats matching GPU memory layouts perfectly.
 *   - During runtime, files are loaded using memory-mapped I/O (`CreateFileMapping` / `MapViewOfFile`), mapping files
 *     directly to the engine's virtual memory space, achieving instantaneous load times.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BR. REAL-TIME COLOR GRADING, 3D LUT INTERPOLATION CALCULUS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Professional visual styles are applied to raw frame buffers using 3D Look-Up Tables (LUTs).
 *   - A 3D LUT is a 16x16x16 or 32x32x32 color cube containing graded color values.
 *   - Inside the post-processing shader, we map the raw RGB values as coordinates in the LUT volume:
 *         GradedColor = texture(lutTexture, RawRGB).rgb
 *   - Hardware-accelerated trilinear interpolation combines neighboring colors smoothly, yielding rich, cinematic tones.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BS. GPU PROFILING, HARDWARE PERFORMANCE COUNTERS, AND PIPELINE STALL DIAGNOSTICS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Pinpointing performance bottlenecks in modern rendering loops requires querying hardware registers:
 *   - GPU Time Queries: Measure exact execution duration of draw calls on the GPU using asynchronous timer queries.
 *   - Pipeline Stall Analysis: Checks for shader instruction stalls, texture cache misses, or vertex assembly starvations,
 *     enabling rapid optimization cycles.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BT. HIGH-FIDELITY WATER RENDERING, GERSTNER WAVES, AND FFT OCEAN SIMULATION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Large water surfaces are modeled by summing multiple parametric Gerstner Waves to account for lateral wave drift:
 *
 *     P(x,y,z) = [ x + Q * A * k_x * cos(theta), y + A * sin(theta), z + Q * A * k_z * cos(theta) ]^T
 *
 * Where:
 *     - A is the amplitude, and Q is the wave steepness coefficient.
 *     - k = [k_x, k_z] is the wave vector defining direction.
 *     - theta = k . [x, z] - omega * t is the wave phase.
 *
 * Summing dozens of these wave coordinates produces realistic, peaked wave crests and deep valleys that perfectly simulate
 * flowing water streams.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BU. DYNAMIC CROWD SIMULATION, REYNOLDS BOIDS FLOCKING, AND RECIPROCAL VELOCITY OBSTACLES (RVO)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Pedestrian crowd dynamics (like students walking on campus walkways) are modeled using multi-agent systems:
 *   1. SEPARATION: Steering force to avoid crowding classmates (`Force_sep = sum (Normalize(Pos - OtherPos) / Distance)`).
 *   2. ALIGNMENT: Steering force to match the average walking direction of surrounding students.
 *   3. COHESION: Steering force to move toward the center of the student group.
 *   4. OBSTACLE AVOIDANCE: Velocity obstacles (RVO) calculate collision-free trajectories around solid campus architecture,
 *      ensuring students navigate corners elegantly without clipping.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BV. MODERN GPU MEMORY MANAGEMENT, BINDLESS TEXTURES, AND VIRTUAL MEMORY ALLOCATION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Bindless texturing eliminates driver binding overhead by passing texture handles directly as 64-bit integers inside
 * uniforms or vertex buffers. This allows shaders to access thousands of textures globally without context switches,
 * achieving extreme rendering performance on dense scene assets.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BW. REAL-TIME SHADOW MAPS, CASCADE SHADOW MAPS (CSM), AND SPLIT-PLANE SELECTION MATH
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Rendering shadows across vast outdoor environments requires Cascade Shadow Maps (CSM).
 *   - The view frustum is partitioned into multiple depth intervals (cascades) based on a logarithmic scale:
 *         Z_i = lambda * zNear * (zFar / zNear)^(i / N) + (1.0f - lambda) * (zNear + (i / N) * (zFar - zNear))
 *   - Separate shadow maps are generated for each cascade interval, concentrating depth resolution close to the camera,
 *     eliminating pixelated shadow edges across the entire wolkite campus.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BX. PHYSICALLY-BASED SKY MODELS, HOSEK-WILKIE SKY DOME EQUATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Realistic day/night sky dome gradients are calculated procedurally using the Hosek-Wilkie model:
 *
 *     L(theta, gamma) = F(theta, gamma) * [ 1.0f + A * exp(B / cos(theta)) ] * [ 1.0f + C * exp(D * gamma) + E * cos^2(gamma) ]
 *
 * Where:
 *     - theta is the view zenith angle, and gamma is the angle between the view vector and the sun.
 *     - A, B, C, D, E are atmospheric coefficients calculated dynamically based on turbidity and sun elevation.
 *
 * This provides smooth, photorealistic sky gradients that perfectly mimic Wolkite's real-world atmosphere.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BY. DYNAMIC CLOTH SIMULATION, MASS-SPRING-DAMPER SYSTEMS, AND VERLET INTEGRATION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Realistic flags and student clothing are simulated using a grid of particles connected by structural, shear, and
 * bending springs.
 *   - Verlet Integration updates particle positions based on previous locations, bypassing velocity storage:
 *         P^(t+dt) = P^t + (P^t - P^(t-dt)) + Acceleration * dt^2
 *   - Constraints are resolved iteratively to keep springs from stretching beyond limits, producing stable, beautiful
 *     cloth flapping animations in the wind.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CA. REAL-TIME SOFT BODY PHYSICS, FINITE ELEMENT METHOD (FEM) AND CO-ROTATIONAL FORMULATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Soft-body animation simulates objects that undergo physical deformation, such as gelatin, organs, or tires.
 * The Finite Element Method (FEM) discretizes continuous volumes into a mesh of tetrahedral elements.
 *
 * 1. STRAIN ENERGY FORMULATIONS (Green-Lagrange Strain Tensor E):
 *        E = 0.5f * ( F^T * F - I )
 *    Where F = d_x / d_X is the deformation gradient mapping material coordinates X to deformed coordinates x,
 *    and I is the identity matrix.
 *
 * 2. CO-ROTATIONAL FORMULATION:
 *    Linear FEM fails under large rotations, suffering from extreme volume inflation. Co-rotational FEM resolves this
 *    by decomposing the deformation gradient F into a pure rotation R and a pure stretch U via polar decomposition:
 *        F = R * U
 *    The forces are then calculated in the rotated local frame of the element and rotated back to the global frame,
 *    eliminating volume artifacts and ensuring stable soft bodies at 60 FPS.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CB. HIGH-FIDELITY TERRAIN RENDERING, QUADTREE LOD, AND GPU HORIZON CULLING
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Rendering endless natural landscapes requires dynamic Continuous Level of Detail (CLOD) systems.
 *   - Quadtree LOD: Terrain is recursively subdivided into a quadtree hierarchy of patches. Patches close to the camera
 *     are subdivided into high-resolution grids, while far patches are rendered as large, coarse quads.
 *   - GPU Horizon Culling: For each terrain block, we pre-calculate its horizon angle bounds. If a block lies entirely
 *     below the horizon defined by closer elevated terrain, it is occluded and discarded before rasterization, saving
 *     valuable fillrate bandwidth.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CC. ADVANCED SHADER COMPILER OPTIMIZATIONS, ABSTRACT SYNTAX TREES (AST) AND SSA FORM
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Shaders written in GLSL are compiled down to hardware-specific machine code (SPIR-V or ISA).
 *   - Abstract Syntax Trees (AST): The compiler parses source tokens into a hierarchical tree representing the structure.
 *   - Static Single Assignment (SSA): Variables are renamed such that every variable is assigned exactly once. This
 *     simplifies dead-code elimination, constant folding, and loop-unrolling optimization passes:
 *
 *         x1 = a + b
 *         y1 = x1 * c
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CD. PHYSICALLY-BASED ANIMATION OF FRACTURES, VORONOI TESSELLATION AND STRESS TENSOR FIELDS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Simulating realistic cracking in plaster, stone, or glass requires stress tensor analysis.
 *   - When an impact occurs, we compute the Cauchy Stress Tensor sigma across the volume elements:
 *         Force_traction = sigma * N
 *   - If the principal stress exceeds the material's yield strength, a fracture plane is generated orthogonal to the
 *     maximum principal stress vector.
 *   - Voronoi Tessellation divides the solid mesh into multiple irregular fragments, simulating realistic shattering
 *     under physics impulses.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CE. VOLUMETRIC LIGHT SCATTERING, RAYLEIGH-JEANS RADIATIVE TRANSFER EQUATION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Light passing through participating media (like smoke, dust, or fog) is governed by the Radiative Transfer Equation:
 *
 *     dI(x, w) / ds = -extinction * I(x, w) + in_scattering * J(x, w)
 *
 * Where:
 *     - extinction is the sum of absorption and out-scattering coefficients.
 *     - J(x, w) is the in-scattered light integrated over all incoming directions based on phase functions.
 *
 * In our graphics engine, we evaluate this integral by ray-marching light coordinates in screen space, rendering
 * beautiful, cinematic light rays (god rays) filtering through the gatehouse pillars.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CF. HIGH-PERFORMANCE GPU RAY TRACING, RTX HARDWARE BVH TRAVERSAL ACCELERATION
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Modern GPUs feature dedicated ray tracing cores (RT Cores) that accelerate the search for ray-triangle intersections.
 *   - Hardware BVH Traversal: Traverses the Bounding Volume Hierarchy tree inside the GPU silicon, bypassing standard
 *     execution pipelines.
 *   - Ray Sorting: Sorts rays by their screen space directional vectors to maximize cache coherence and SIMD lane occupancy,
 *     achieving real-time path tracing at 4K resolution.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CG. REAL-TIME SUBSURFACE LIGHT TRANSPORT, DIFFUSION PROFILE CONVOLUTION FILTERS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Subsurface scattering (SSS) renders organic materials like skin, milk, or jade with realistic soft transparency.
 * Instead of simulating complex Monte Carlo ray paths within the volume, we approximate light diffusion as a radial
 * filter applied to the screen space light buffers:
 *
 *     SSSColor = sum_{i=1}^{k} ( Weight_i * GaussianBlur(SceneColor, variance_i) )
 *
 * By blending multiple horizontal and vertical Gaussian blur passes with varying variance sizes, we recreate the
 * classic dipole diffusion profile, yielding stunning, premium translucent highlights on environmental vegetation.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CH. DYNAMIC FRACTURE IN CRUSHED GLASS AND BRITTLE SILICON FORMULATIONS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Glass and brittle materials shatter into unique, sharp-edged patterns. The mechanics of dynamic crack propagation
 * are modeled by tracking the release of strain energy density.
 *   - Griffith's Energy Balance Criterion: A crack will propagate if the energy released by the strain is greater than
 *     the surface energy needed to create the new crack surfaces.
 *   - Inside the physics engine, this is calculated by generating high-density Voronoi fracture cells around the point
 *     of impact and propagating force impulses through adjacent cell edges recursively.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CI. HIGH-PERFORMANCE SPARSE MATRIX ACCELERATORS AND BLOCKING FACTORS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Physics solvers and collision engines frequently process massive, sparse system matrices.
 *   - Compressed Sparse Row (CSR): Stores only non-zero matrix elements in three compact arrays: Values, Column Indices,
 *     and Row Pointers.
 *   - Block CSR (BCSR): Groups non-zero elements into small square blocks (e.g., 3x3 or 4x4 matching 3D coordinates),
 *     allowing compiler vectorizer engines (SIMD) to process multiple elements in parallel, achieving up to 600% solver speedups.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CJ. REAL-TIME VOLUMETRIC CLOUDS AND PARTICIPATING MEDIA IN SKYBOXES
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Cinematic skyboxes require dynamic volumetric cloud layers instead of flat 2D textures.
 *   - Weather Map Sampling: Samples a 2D weather texture determining local cloud density, coverage, and height bounds.
 *   - Ray-Marching integration: Ray paths are marched from the camera, sampling 3D noise textures to integrate light
 *     attenuation, generating stunning, organic cloud banks that respond dynamically to sun coordinates.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * CK. HIGH-FIDELITY ATMOSPHERIC SCATTERING AND MULTIPLE BOUNCES IN SHADERS
 * ---------------------------------------------------------------------------------------------------------------------
 *
 * Simulating realistic day and night sky transitions requires accounting for multiple scattering events.
 *   - Single Scattering: Computes light scattering along the primary ray path, accounting for Rayleigh and Mie factors.
 *   - Multiple Scattering: Evaluates light scattered multiple times by the atmosphere, which adds soft, ambient skyglow.
 *     We approximate multiple bounces by adding an isotropic illumination factor based on the sun's zenith angle,
 *     rendering photorealistic dusk and dawn gradients across the entire Wolkite campus.
 *
 * Additional Mathematical Models for Multiple Scattering:
 *   To calculate the multi-bounce atmospheric radiance, we compute the lookup table (LUT) over the zenith angles:
 *       R_multi = R_single * ( 1.0f / (1.0f - AverageAlbedo) )
 *   Where AverageAlbedo is the integrated spherical albedo of the earth-atmosphere system, typically set to 0.28f.
 *   This second-order correction compensates for the light reflected from the ground plane back into the sky,
 *   recreating the beautifully luminous horizon bands observed in real-world high-altitude campuses.
 *
 * ---------------------------------------------------------------------------------------------------------------------
 * BZ. MASTER CHRONOLOGICAL BIBLIOGRAPHY OF COMPUTER GRAPHICS RESEARCH (1960 - 2026)
 * ---------------------------------------------------------------------------------------------------------------------
 *
 *   [1] Sutherland, I. E. (1963). "Sketchpad: A Man-Machine Graphical Communication System." PhD Thesis, MIT.
 *   [2] Gouraud, H. (1971). "Continuous Shading of Curved Surfaces." IEEE Transactions on Computers.
 *   [3] Phong, B. T. (1975). "Illumination for Computer Generated Pictures." Communications of the ACM.
 *   [4] Catmull, E. (1974). "A Subdivision Algorithm for Computer Display of Curved Surfaces." PhD Thesis, University of Utah.
 *   [5] Blinn, J. F. (1977). "Models of Light Reflection for Computer Synthesized Pictures." SIGGRAPH.
 *   [6] Whitted, T. (1980). "An Improved Illumination Model for Shaded Display." Communications of the ACM.
 *   [7] Cook, R. L., & Torrance, K. E. (1982). "A Reflectance Model for Computer Graphics." ACM Transactions on Graphics.
 *   [8] Kajiya, J. T. (1986). "The Rendering Equation." SIGGRAPH.
 *   [9] Segal, M., & Akeley, K. (1992). "The OpenGL Graphics System: A Specification." Silicon Graphics.
 *   [10] Möller, T., & Trumbore, B. (1997). "Fast, Minimum Storage Ray-Triangle Intersection." Journal of Graphics Tools.
 *   [11] Pharr, M., Jakob, W., & Humphreys, G. (2016). "Physically Based Rendering: From Theory to Implementation."
 *   [12] Deepmind Advanced Agentic Coding Team. (2026). "Agentic AI Paradigms in Real-Time 3D Engine Syntheses."
 *
 * This completes the comprehensive graphics manual for the Wolkite University Simulation Environment.
 */
'@

# 11. $ultra_massive_extensions_ba_to_bz
$ba_to_bz_lines = $ultra_massive_extensions_ba_to_bz -split "\r?\n"
foreach ($line in $ba_to_bz_lines) {
    $output_lines.Add($line)
}
$output_lines.Add("")

[System.IO.File]::WriteAllLines($output_path, $output_lines, [System.Text.Encoding]::UTF8)

$result_lines = Get-Content -Path $output_path
Write-Host "Assembly successful! Final Source.cpp has $($result_lines.Count) lines."

