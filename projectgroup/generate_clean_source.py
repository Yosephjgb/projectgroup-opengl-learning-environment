import os

def main():
    original_path = r"c:\Users\hp\source\repos\projectgroup\projectgroup\original_Source_utf8.cpp"
    output_path = r"c:\Users\hp\source\repos\projectgroup\projectgroup\Source.cpp"

    if not os.path.exists(original_path):
        print(f"Error: Original source file not found at {original_path}")
        return

    with open(original_path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Define beautiful header and documentation sections
    header = """/**
 * =================================================================================================
 *                               WOLKITE UNIVERSITY CAMPUS SIMULATION
 * =================================================================================================
 *
 *                                  ______   __    __   ________
 *                                 /      \\ /  |  /  | /        |
 *                                /$$$$$$  |$$ |  $$ | $$$$$$$$/
 *                                $$ |  $$ |$$ |  $$ | $$ |__
 *                                $$ |  $$ |$$ |  $$ | $$    |
 *                                $$ |  $$ |$$ |  $$ | $$$$$/
 *                                $$ \\__$$ |$$ \\__$$ | $$ |_____
 *                                $$    $$/ $$    $$/  $$       |
 *                                 $$$$$$/   $$$$$$/   $$$$$$$$/
 *
 * =================================================================================================
 * A Premium, High-Fidelity 3D Virtual Reconstruction of the Wolkite University Gatehouse Complex
 * and its Immersive, Live-Action Classroom Interior. Built using Modern OpenGL and GLUT.
 * =================================================================================================
 *
 * -------------------------------------------------------------------------------------------------
 * 1. MATHEMATICAL DEEP DIVE & GRAPHICS EQUATIONS
 * -------------------------------------------------------------------------------------------------
 *
 * A. PERSPECTIVE VIEWING FRUSTUM PROJECTION
 * The 3D world space is mapped into 2D normalized device coordinates (NDC) using a perspective
 * projection matrix, commonly initialized via gluPerspective(fovY, aspect, zNear, zFar).
 * The mathematical matrix applied to the matrix stack is:
 *
 *     [   f / aspect         0                 0                      0             ]
 *     [       0              f                 0                      0             ]
 *     [       0              0      (zFar+zNear)/(zNear-zFar)  (2*zFar*zNear)/(zNear-zFar) ]
 *     [       0              0                -1                      0             ]
 *
 * Where:
 *     f = cot(fovY / 2) = 1.0 / tan(fovY / 2)
 *     aspect = viewport_width / viewport_height
 *     zNear = Distance to the near clipping plane (typically 0.1f)
 *     zFar = Distance to the far clipping plane (typically 1000.0f)
 *
 * The projection transforms three-dimensional view space points [Xe, Ye, Ze, 1]^T to homogenous
 * clip coordinates [Xc, Yc, Zc, Wc]^T. Division by Wc (where Wc = -Ze) yields normalized device
 * coordinates, producing the natural perspective scaling where distant objects appear smaller:
 *
 *     Xndc = Xc / Wc = -f * Xe / (aspect * Ze)
 *     Yndc = Yc / Wc = -f * Ye / Ze
 *
 * B. PHONG REFLECTION AND LIGHTING MODEL
 * Lighting calculations determine color value at any given vertex using a three-component formula:
 * Ambient, Diffuse, and Specular reflection.
 *
 *     I_total = I_ambient + I_diffuse + I_specular
 *
 * Where:
 *     1. Ambient Component (Constant scattered lighting):
 *        I_ambient = Ka * La
 *        Where Ka is the material's ambient coefficient, La is the light source ambient intensity.
 *
 *     2. Diffuse Component (Lambertian Reflection based on angle of light):
 *        I_diffuse = Kd * Ld * max(0.0, N . L)
 *        Where Kd is the material diffuse color, Ld is light source diffuse intensity, N is the unit
 *        surface normal vector, and L is the unit light direction vector pointing from vertex to light.
 *
 *     3. Specular Component (Glossy highlight based on reflection and view direction):
 *        I_specular = Ks * Ls * (max(0.0, R . V))^alpha
 *        Where Ks is specular reflectivity coefficient, Ls is specular light source intensity, R is the
 *        reflection vector (R = 2 * (N . L) * N - L), V is the unit view direction pointing from vertex
 *        to camera, and alpha is the material shininess exponent (specular shininess, set to 64 here).
 *
 * C. MODELVIEW COORDINATE TRANSFORMATIONS
 * Rigid-body movements are mathematically represented by 4x4 matrix multiplications. In OpenGL,
 * transformation matrices are applied sequentially in a column-major format.
 *
 *     1. Translation Matrix (Positioning):
 *        T(tx, ty, tz) = [ 1  0  0  tx ]
 *                        [ 0  1  0  ty ]
 *                        [ 0  0  1  tz ]
 *                        [ 0  0  0  1  ]
 *
 *     2. Rotation Matrix around Y-axis (Yaw Orientation):
 *        Ry(theta) = [  cos(theta)  0  sin(theta)  0 ]
 *                    [      0       1      0       0 ]
 *                    [ -sin(theta)  0  cos(theta)  0 ]
 *                    [      0       0      0       1 ]
 *
 *     3. Scaling Matrix (Dimensions):
 *        S(sx, sy, sz) = [ sx   0    0   0 ]
 *                        [  0  sy    0   0 ]
 *                        [  0   0   sz   0 ]
 *                        [  0   0    0   1 ]
 *
 * Matrix compositions are non-commutative: M = T * R * S. This sequence ensures objects are first
 * scaled in local coordinates, rotated around their local origin, and finally translated to their
 * absolute world coordinates. Reverse order results in wildly incorrect eccentric translations.
 *
 * D. PARAMETRIC ELLIPTIC PATHING
 * Walking animations on the gatehouse balcony trace an elliptical path. The position of each student
 * at walk phase theta is governed by parametric equations:
 *
 *     x(theta) = A * sin(theta)
 *     z(theta) = Z_center + B * cos(theta)
 *
 * To determine the correct yaw facing angle so the student faces the walking direction tangent,
 * we take derivatives with respect to theta:
 *
 *     dx/d(theta) = A * cos(theta)
 *     dz/d(theta) = -B * sin(theta)
 *
 * The yaw rotation angle (facing angle) is then calculated using the arctangent function:
 *
 *     yaw = atan2(dx/d(theta), dz/d(theta)) * (180.0 / PI)
 *
 * -------------------------------------------------------------------------------------------------
 * 2. SYSTEM ARCHITECTURE & INTERACTIVES
 * -------------------------------------------------------------------------------------------------
 * KEYBOARD INTERACTION SYSTEM:
 *   - 'W' / 'S' Keys: Adjust camera height levels (`camHeight`), enabling vertical panorama panning.
 *   - 'N' / 'N' Keys: Triggers full Day/Night lighting and material overrides. Overrides ambient,
 *     diffuse, and clear-color values inside the lighting calculation routine (`applySkySettings`).
 *   - 'G' / 'G' Keys: Activates the cinematic classroom entrance transition sequence. Switches the system
 *     mode to `MODE_TRANSITION`, triggering an automatic camera sweep and smooth interpolation.
 *   - 'B' / 'B' Keys: Initiates the exit sequence from the classroom back to the exterior gatehouse environment.
 *   - Arrow Keys: Manages horizontal translations (`posX`, `posZ`) and rotation yaw (`autoAngle`) for
 *     interactive, first-person navigation inside the high-fidelity interior building space.
 *
 * COLLISION AND BOUNDARY DETECTION:
 * The interior environment is strictly locked within virtual collision walls to prevent the first-person
 * camera from leaving the room boundary:
 *   - posX: [-3.30f, 3.30f]
 *   - posZ: [-6.30f, 0.45f]
 *   - camHeight: [0.40f, 1.80f] (mimicking authentic human height parameters)
 */
"""

    solidbox_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR SOLIDBOX GEOMETRY
 * =================================================================================================
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
 */
"""

    drawtree_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR PROCEDURAL FORESTRY
 * =================================================================================================
 * The `drawTree` function renders realistic vegetation types using translation and rotation composition.
 *
 * A. CYLINDER AND CONE TRANSLATIONS
 * 1. Pine/Evergreen: Consists of a cylindrical trunk and a conical leaf crown.
 *    The conical canopy is drawn using `glutSolidCone(baseRadius, height, slices, stacks)`.
 *    Because `glutSolidCone` by default projects vertically along the Z-axis, it is rotated by -90
 *    degrees around the X-axis (`glRotatef(-90, 1, 0, 0)`) to stand upright along the Y-axis.
 *
 * 2. Leafy/Deciduous: Uses a spherical canopy drawn using `glutSolidSphere`.
 *    Translating the canopy to the top of the trunk ($Y = 2.2f$) simulates standard biological heights.
 *
 * B. REDUCING VERTEX OVERHEAD
 * Slices and stacks are intentionally set to moderate values (e.g., 8 slices, 4 stacks) to keep
 * rendering calculations extremely fast, avoiding system stuttering when populating large forests of
 * 100+ trees.
 */
"""

    cobblestone_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR COBBLESTONE GENERATION
 * =================================================================================================
 * Rendering realistic paths requires procedural texture synthesis to avoid flat, lifeless gray polygons.
 * `drawCobblestonePath` generates individual physical stones on a grid with micro-variations.
 *
 * A. DETERMINISTIC PSEUDO-RANDOM SEEDING
 * Because OpenGL is stateless and displays frames 60 times a second, we cannot use typical standard
 * random functions like `rand()`, which would cause cobblestones to flicker uncontrollably. Instead,
 * we generate a deterministic "seed" based on the stone's absolute coordinate system position:
 *
 *     seed = (int)((x + 50.0f) * 10.0f) * 131 + (int)((z + 50.0f) * 10.0f)
 *
 * B. NOISE-BASED DISPLACEMENT (JITTER)
 * Using this seed, we apply small, pseudo-random offsets to each stone's position (jitter):
 *
 *     jx = (seed % 7) * 0.01f
 *     jz = (seed % 9) * 0.01f
 *
 * C. ORGANIC SPACING AND SHADING
 *   1. Skip Pattern: We omit roughly 10% of stones (`if (seed % 11 == 0) continue`) to simulate broken
 *      paving where vegetation or grass peaks through.
 *   2. Color Gradients: We vary the gray tones by adding a small value derived from our deterministic seed:
 *      v = (seed % 30) / 100.0f. This breaks up geometric monotony, mimicking natural weathering.
 */
"""

    bush_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR LANDSCAPE SHRUBBERY
 * =================================================================================================
 * Continuous solid meshes fail to capture the lush foliage of real shrubbery. The `drawBush` function
 * constructs a dense organic bush by clustering five overlapping spheres.
 *
 * A. SPHERICAL CLUSTERING PATTERN
 * One central sphere is offset vertically to serve as the crown, surrounded by four secondary spheres
 * oriented in cardinal directions (North, South, East, West) with minor translation offsets:
 *   - Center:  [  0.00f,  0.25f,  0.00f ]
 *   - North:   [  0.15f,  0.20f,  0.12f ]
 *   - South:   [ -0.15f,  0.20f,  0.12f ]
 *   - East:    [  0.12f,  0.18f, -0.15f ]
 *   - West:    [ -0.12f,  0.18f, -0.15f ]
 *
 * B. SCENIC TEXTURING
 * The overlapping spheres create a highly complex silhouette that reacts dynamically to ambient and
 * diffuse lights, creating a highly organic foliage texture.
 */
"""

    grass_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR VEGETATION VEINS
 * =================================================================================================
 * To supplement structural bushes, `drawLongGrass` populates the path-road intersections with physical
 * grass blades.
 *
 * A. PRIMITIVE LINE STRIPS
 * Rather than processing massive polygonal meshes, each blade is drawn using standard C++ primitive lines
 * (`GL_LINES`). Vertices start on the ground plane ($Y = 0.0f$) and extend slightly outwards, adding
 * natural, organic curvatures.
 *
 * B. SHADING GRADIENTS
 * A deep, dark yellowish-green color (`0.35f, 0.55f, 0.25f`) is selected, which provides high visual
 * contrast when peeking through the lighter cobblestone tiles or curbs.
 */
"""

    ditch_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR CONCRETE DRAINS
 * =================================================================================================
 * Real-world university campuses include infrastructure for water drainage. `drawDitch` models a sloped
 * concrete drain running parallel to the walkways.
 *
 * A. SLANTED SURFACE NORMAL MATH
 * Because the sides of a triangular ditch are sloped, standard cardinal normals (like [0, 1, 0]) are
 * mathematically invalid and will ruin the lighting shading. We calculate correct normals by taking
 * cross products of the edge vectors.
 *
 * For a sloped plane rising by height delta `d` over width `w`:
 *   - Left Slope Normal:  N = [  0.447f,  0.894f,  0.0f ]
 *   - Right Slope Normal: N = [ -0.447f,  0.894f,  0.0f ]
 *
 * B. VISUAL INTEGRATION
 * Using concrete colors blended with organic mossy green-gray tones (`0.42f, 0.48f, 0.38f`) integrates
 * the drainage ditch beautifully with both the cobblestone paths and the surrounding lawn hedges.
 */
"""

    ditchhoriz_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR HORIZONTAL CONCRETE DRAINS
 * =================================================================================================
 * This is the horizontally aligned variant of the concrete drainage ditch, oriented along the X-axis
 * in front of the main building base.
 *
 * A. HORIZONTAL NORMAL CALCULATIONS
 * Similar to the vertical ditch, we calculate normals adjusted for slope, but oriented along the Z-axis:
 *   - Back Slope Normal:  N = [  0.0f,  0.894f,  0.447f ]
 *   - Front Slope Normal: N = [  0.0f,  0.894f, -0.447f ]
 *
 * B. GEOMETRIC BOUNDS
 * The horizontal ditch perfectly borders the concrete foundation plinth, physically separating the lawn
 * landscaping from the main neoclassical academic building.
 */
"""

    door_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR DETAILED ARCHITECTURAL DOOR ASSEMBLIES
 * =================================================================================================
 * In high-fidelity architectural simulations, doors must be fully functional moving assets rather than
 * flat textured rectangles.
 *
 * A. TRANSOM WINDOW ASSEMBLY
 * The door assembly features a solid dark glass transom window at the top, a structural white concrete
 * frame, and a dark interior "mask box" behind the door. This mask box is completely black and sits
 * slightly behind the door plane to hide the wall geometry and create a realistic depth illusion when the
 * door rotates open.
 *
 * B. TRANSLATION & ROTATION MATRIX COMPOSITION (THE HINGE HACK)
 * Standard rotations inside OpenGL are performed relative to the local origin ([0,0,0]). If we rotate the
 * door directly, it will spin around its center, which is completely incorrect for a swinging door.
 * To hinge the door at its left edge ($X = x - w/2$), we perform a compound translation-rotation sequence:
 *
 *     1. Translate Hinge to Origin:
 *        glTranslatef(x - w / 2, y, z)
 *     2. Rotate Hinge:
 *        glRotatef(openAngle, 0, 1, 0)
 *     3. Translate Hinge Back:
 *        glTranslatef(-(x - w / 2), -y, -z)
 *
 * C. MICRO-GEOMETRIC ADDITIONS
 * The door face contains multiple decorative vertical ridges (`glBegin(GL_LINES)`), a raised horizontal
 * middle bar (`solidBox`), and a metallic dark-grey handle block (`glutSolidCube`).
 */
"""

    student_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR DYNAMIC BIOLOGICAL CHARACTERS (STUDENTS & TEACHERS)
 * =================================================================================================
 * Human figures in standard simulations are often simple cylinders or static meshes. To make the campus
 * feel alive, `drawStudent` implements a fully articulated, multi-jointed C++ anatomical model that
 * supports realistic walking animations and interactive seated states.
 *
 * A. MOTION PHYSICS & WAVE EQUATIONS
 * Dynamic walking movements (arm swings, leg swings, and head bobbing) are generated procedurally in
 * real time using periodic sine and cosine wave equations.
 *
 *   1. Head/Body Bobbing:
 *      bob = abs(sin(walkPhase * 2.0)) * 0.05
 *      This simulates the vertical drop and rise of the human torso during a normal stride cycle, preventing
 *      characters from looking like they are floating or sliding.
 *
 *   2. Leg Swing Angle (Alternating strides):
 *      legSwing = sin(walkPhase) * 35.0 degrees
 *      One leg rotates forward by `legSwing` while the other rotates backward by `-legSwing`.
 *
 *   3. Arm Swing Angle (Natural walking balance):
 *      armSwing = sin(walkPhase) * 45.0 degrees
 *      Arms swing in opposition to the legs: the left arm swings forward while the left leg swings backward.
 *
 * B. INTERACTIVE STATE MAPPINGS
 *   1. Seated State (`isSeated = true`):
 *      - Leg joints rotate forward by 90 degrees around the X-axis, and knees bend backward by 90 degrees.
 *      - Arm joints rotate forward by 30 degrees, resting realistically on the student desks.
 *      - Bobbing physics is deactivated (`bob = 0.0f`).
 *
 *   2. Teaching/Lecturing State (`isTeaching = true`):
 *      - The character is scaled up by 15% to simulate a mature lecturer standing at a podium.
 *      - The walking trajectory is slowly paced across the front whiteboard area (`tx += sin(walkPhase * 0.05) * 0.6`).
 *      - The teacher periodically rotates 180 degrees to face the whiteboard when the phase sine wave peaks,
 *        mimicking writing notes.
 *
 * C. MULTI-VARIANT VISUAL INDIVIDUALITY
 * To avoid drawing an identical army of clones, color palettes (shirts, pants, hair) and heights are
 * derived procedurally from the student's unique ID (`variant`):
 *   - sCol = variant % 6 (6 distinct shirt colors)
 *   - hCol = variant % 4 (4 distinct hair colors: black, brown, blonde, gray)
 */
"""

    students_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR STUDENT BEHAVIOR MANAGER
 * =================================================================================================
 * The `drawStudents` function acts as a high-level behavior manager, controlling the coordinate systems,
 * walking paths, and animation states of all students roaming the campus.
 *
 * A. ELLIPTICAL STRIDERS
 * Students walking along the first-floor balcony traverse a parametric ellipse. By offsetting their
 * phase parameters (`phaseOffset = i * 120.0`), they remain evenly spaced along the path.
 *
 * B. CINEMATIC INTEGRATION
 * During the classroom entrance sequence (`currentMode == MODE_TRANSITION`), Student 0 dynamically detaches
 * from the standard exterior walkway and transitions to the entrance path, walking through the center
 * double doors to take their seat inside the classroom.
 */
"""

    classroom_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR HIGH-FIDELITY CLASSROOM INTERIORS
 * =================================================================================================
 * The interior classroom is the visual centerpiece of this simulation. It renders a modern, active
 * educational room fully populated with desks, seating, and high-fidelity props.
 *
 * A. TILE ARCHITECTURE AND GRID WALLS
 *   1. Floor Tiles: Rendered with a high-contrast tile grid to create perspective depth.
 *   2. Two-Tone Walls: Adheres to university design rules, using a dark gray concrete base band
 *      up to $Y = 1.2f$, topped with clean white plaster walls.
 *   3. Geometric Ceiling Beams: Heavy structural concrete beams are positioned at the front
 *      of the room to support ceiling loads and provide realistic shadow depth.
 *
 * B. EDUCATIONAL INSTRUMENTS AND MULTIMEDIA PROPS
 *   1. Analog Wall Clock: Positioned directly above the whiteboard, with tick marks and hands.
 *   2. Educational Posters: Features a Periodic Table and a World Map rendered procedurally with
 *      colored pixel matrices on the front wall.
 *   3. Multimedia Screen & TV: A large flatscreen TV sits on the right side of the front wall,
 *      complete with a soft light-blue screen glow.
 *   4. Ceiling Projector: A modern projector hangs from the ceiling, complete with a mounting pole,
 *      lens cylinder, and light-blue lens glass.
 *   5. Air Conditioning Unit: Features vents, intake grills, and branding, adding realistic modern details.
 *
 * C. DESK ARRAYS
 * Features 42 student desks arranged in a 6-column by 7-row grid. Each desk has a small notebook prop,
 * and a seated student model with unique procedurally generated colors.
 */
"""

    stars_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR PROCEDURAL STARFIELDS
 * =================================================================================================
 * At night, the sky renders a detailed field of 200 twinkling stars.
 *
 * A. DETERMINISTIC RANDOM SEEDING
 * Similar to the cobblestones, we use a deterministic coordinate seeding algorithm to ensure stars
 * remain at fixed coordinate locations rather than moving around randomly:
 *
 *     seed = i * 1973 + 42
 *     px = fmod(seed * 0.0137f, 160.0f) - 80.0f
 *
 * B. TWINKLING SHADING
 * To simulate atmospheric twinkling, the brightness of each star is slightly varied based on its
 * deterministic coordinate position, giving the night sky a natural, sparkling look.
 */
"""

    moon_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR DYNAMIC CRESCENT MOONS
 * =================================================================================================
 * The night sky contains a detailed 3D crescent moon.
 *
 * A. DYNAMIC ORBITAL MATRICES
 * The moon's position is calculated using the system's global day/night cycle angle (`sunMoonAngle`):
 *
 *     glRotatef(angle, 0, 0, 1)
 *     glTranslatef(0.0f, 30.0f, -60.0f)
 *
 * This rotates the coordinate frame around the Z-axis, creating a realistic rising and setting orbit.
 *
 * B. CRESCENT SHADOWING OVERLAYS
 * Instead of relying on complex 3D meshes, a classic crescent shape is achieved by rendering a larger,
 * pale yellow-white sphere, followed by a slightly smaller, dark-blue sphere slightly offset in
 * coordinate space. This masks out the backside of the moon, producing a beautiful, high-fidelity
 * crescent phase.
 */
"""

    sun_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR SUNBURST GLOW ASSEMBLIES
 * =================================================================================================
 * The daytime sky features a bright, glowing sunburst assembly.
 *
 * A. PROCEDURAL RAY GEOMETRY
 * Features 16 procedural sunrays rendered using a triangle loop (`GL_TRIANGLES`).
 *
 * B. ROTATIONAL ALIGNMENT
 * Ray vertices are rotated around the sun's center using sine and cosine trigonometry:
 *
 *     a = i / numRays * 2 * PI
 *     a1 = a - rayHalfW / innerRadius
 *     a2 = a + rayHalfW / innerRadius
 *
 * C. RADIAL COLOR GRADIENTS
 * Vertices fade from a bright, opaque yellow at the core (`1.0f, 0.95f, 0.3f, 0.9f`) to a completely
 * transparent orange at the tip (`1.0f, 0.75f, 0.1f, 0.0f`), simulating a realistic solar flare.
 */
"""

    cloud_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR DYNAMIC CLOUD GENERATOR
 * =================================================================================================
 * The sky features five fluffy clouds drifting slowly over time.
 *
 * A. PROCEDURAL SPHERE CLUSTERING
 * Each cloud is constructed by clustering seven overlapping spheres of varying radii, offset along
 * the X, Y, and Z axes to create a realistic, puffy silhouette.
 *
 * B. LIGHTING STATE SENSITIVITY
 * Cloud colors react dynamically to the day/night cycle. In the day, they are bright white; at night,
 * they shift to a dark blue-gray, blending realistically with the ambient light.
 */
"""

    sky_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR SKYBOX ARCHITECTURE
 * =================================================================================================
 * The `drawSky` function serves as the container for sky objects, managing depth buffers and lighting.
 *
 * A. DEPTH AND LIGHT DISABLING
 * Before drawing any sky elements, lighting is temporarily disabled (`glDisable(GL_LIGHTING)`).
 * This ensures stars, clouds, the sun, and the moon are drawn with their direct emissive colors rather
 * than being shaded by coordinate light sources.
 *
 * B. SYSTEM SWITCHING
 * Depending on the state of `isNight`, the system automatically switches between the night-sky renderer
 * (stars, moon) and the day-sky renderer (sun), creating a seamless transition.
 */
"""

    car_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR VEHICLE CHASSIS ASSEMBLIES
 * =================================================================================================
 * To make the campus environment feel dynamic, `drawCar` models a modern, moving vehicle.
 *
 * A. RIGID BODY ASSEMBLY
 * The vehicle is built using five distinct solid boxes:
 *   1. Lower Chassis (black base)
 *   2. Upper Cabin (windshield and roof structure)
 *   3. Wheels (four cylinders with silver circular rims)
 *   4. Headlights (glowing yellow/white blocks)
 *   5. Taillights (opaque red safety blocks)
 *
 * B. TRANSLATION MATRIX PROGRESSION
 * The car moves along the asphalt road ($Z = 14.0f$) at a constant rate, resetting when it reaches
 * the edge of the campus plane, creating a continuous loop.
 */
"""

    earth_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR GROUND ENVIRONMENT ARCHITECTURE
 * =================================================================================================
 * The `drawEarth` function renders the main environment, laying down the grass lawn, asphalt road,
 * lane markings, cobblestone paths, concrete plinths, and landscaping.
 *
 * A. COBBLESTONE LAYOUT AND ALIGNMENT
 * Cobblestone paths are laid out on either side of the main building base ($X = -9.25f$ and $X = 9.25f$).
 *
 * B. SHRUBBERY AND DENSE HEDGES
 * A continuous hedge row is built using a tight loop that places bushes every $0.60f$ units around the
 * rectangular lawn. Inside the hedge, long grass blades are randomly dispersed using a noise-jitter
 * coordinate offset.
 *
 * C. FORESTRY DISTRIBUTION
 * Dense avenues of leafy pine trees are planted along the pathways, casting realistic shadows and
 * framing the central academic gateway.
 */
"""

    staircase_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR DOG-LEG STAIRCASE COMPLEX
 * =================================================================================================
 * Models the U-shaped (dog-leg) structural staircases flanking either side of the central gatehouse.
 *
 * A. FLIGHT SLOPE TRIGONOMETRY
 * Treads and risers are mathematically calculated to bridge the vertical height from the ground plinth
 * ($Y = 0.15f$) up to the first-floor balcony ($Y = 1.00f$).
 *
 *   1. Flight 1 (Ground to mid-landing landing):
 *      Treads rise from Y=0 to Y=0.51 over Z-axis values, using 6 stepping segments.
 *   2. Mid-Landing Platform:
 *      A wide flat concrete slab at $Y = 0.51f$, serving as the dog-leg turning point.
 *   3. Flight 2 (Mid-landing to top balcony):
 *      Treads rise from Y=0.51 to Y=1.00, climbing in the opposite direction.
 *
 * B. BALUSTER DENSITY PATTERNS
 * Railing posts (balusters) are distributed along the stairs at a high density ($0.06f$ units).
 * Top handrails are drawn as thick sloped line strips (`GL_LINES` with `glLineWidth(4.5f)`) that perfectly
 * parallel the slope of each stair flight.
 *
 * C. STRUCTURAL UTILITIES
 * Includes concrete drain pipes and collection blocks next to the stairs to route rainwater runoff,
 * adding high-fidelity architectural realism.
 */
"""

    building_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR NEOCLASSICAL ACADEMIC GATEHOUSE
 * =================================================================================================
 * The gatehouse is the primary architectural structure of the campus exterior. It is styled in a
 * classical yellow-tan and white theme with a massive rear sloping roof section.
 *
 * A. STRUCTURAL LEVEL SEPARATIONS
 *   1. Foundation Plinth: A massive stone base that supports the entire building.
 *   2. Access Ramp: A slanted concrete ramp for accessibility, complete with sloped side triangles.
 *   3. Ground Floor: Features clean white walls and five detailed Tan double doors.
 *   4. First Floor: Features an open balcony corridor with railings and five matching entry doors.
 *   5. Upper Attic: Features tan brickwork with rows of tiny decorative air holes.
 *
 * B. REAR TRAPEZOIDAL SECTION
 * A massive rear structure with sloped side walls and five large window segments.
 *
 * C. STRUCTURAL SUPPORT MATRIX
 * Six heavy concrete pillars support the front roof canopy, extending from the ground plinth up to the
 * roof trusses at $Y = 2.45f$.
 */
"""

    display_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR REAL-TIME DISPLAY SYSTEM
 * =================================================================================================
 * The `display` function is the primary entry point for the frame rendering loop, called 60 times a
 * second by the graphics system.
 *
 * A. RIGID FIRST-PERSON CAMERA LOCKDOWN
 * To prevent the camera from clipping through wall geometry, boundary checks are applied to the camera's
 * position before any rendering calculations occur.
 *
 * B. CINEMATIC INTERPOLATION MATH
 * Transitions between the exterior and interior views use smooth step interpolation (Hermite curves):
 *
 *     easeP = p * p * (3.0f - 2.0f * p)
 *
 * This produces an elegant, ease-in-ease-out camera sweep that feels incredibly premium.
 *
 * C. MATRIX STACK OPERATIONS
 * Coordinate transformations are pushed and popped from the matrix stack using `glPushMatrix()` and
 * `glPopMatrix()` to ensure rendering transformations on individual objects do not bleed into the
 * global coordinate space.
 */
"""

    skybox_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR DYNAMIC SKYBOX SHADING Presets
 * =================================================================================================
 * The `applySkySettings` function controls the global lighting parameters of the simulation.
 *
 * A. AMBIENT, DIFFUSE, AND SPECULAR LIGHT COMPONENT MATRICES
 * Depending on the state of `isNight`, lighting parameters are updated dynamically:
 *
 *   1. Daytime Settings:
 *      - Sky Clear Color: Bright sky blue (`0.6f, 0.8f, 0.92f`)
 *      - Ambient Color: Soft white scatter (`0.4f, 0.4f, 0.4f`)
 *      - Diffuse Color: Strong daylight sun (`0.9f, 0.9f, 0.9f`)
 *      - Light Position: High side vector (`-15.0f, 25.0f, 25.0f`)
 *
 *   2. Nighttime Settings:
 *      - Sky Clear Color: Deep midnight blue (`0.05f, 0.07f, 0.15f`)
 *      - Ambient Color: Cool night scatter (`0.08f, 0.08f, 0.15f`)
 *      - Diffuse Color: Soft moonlight (`0.15f, 0.15f, 0.25f`)
 *      - Light Position: High moon vector (`0.0f, 25.0f, 10.0f`)
 */
"""

    init_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR SYSTEM INITIALIZATION
 * =================================================================================================
 * The `init` function sets up the OpenGL rendering state machine, depth buffers, material properties,
 * and antialiasing parameters.
 *
 * A. DEPTH BUFFER & SHADING
 *   1. Depth Buffer: Enabled via `glEnable(GL_DEPTH_TEST)` to ensure objects behind are correctly obscured.
 *   2. Shading: Smooth shading (`glEnable(GL_SMOOTH)`) enables beautiful color interpolation across faces.
 *
 * B. ANTI-ALIASING
 * Anti-aliasing is enabled using alpha blending (`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`)
 * and line smoothing (`glHint(GL_LINE_SMOOTH_HINT, GL_NICEST)`) to eliminate jagged edges on railings,
 * stairs, and wires.
 */
"""

    reshape_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR VIEWPORT RESHAPING
 * =================================================================================================
 * The `reshape` function keeps the 3D scene from stretching or distorting when the window is resized.
 *
 * A. ASPECT RATIO PRESERVATION
 * Updates the perspective matrix with the new window width and height to preserve the correct aspect ratio.
 *
 * B. VIEWPORT MAPPING
 * Sets the viewport bounds to match the new window dimensions, scaling the rendering output cleanly.
 */
"""

    keyboard_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR USER KEYBOARD TRANSLATION HANDLE
 * =================================================================================================
 * Handles normal key presses (letters and numbers), mapping keys to system variables like camera height
 * or transition states.
 *
 * A. HEIGHT CONTROLS
 * Adjusts camera height with speed limits to keep the user from flying off the map.
 *
 * B. CYCLE TRIGGERING
 * Toggles Day/Night modes and triggers the cinematic classroom entrance transition sequence.
 */
"""

    specialkeys_theory = """/**
 * =================================================================================================
 * THEORY & MATHEMATICAL MODEL FOR USER SPECIAL KEY TRANSLATION HANDLE
 * =================================================================================================
 * Handles special key presses like arrow keys to control the first-person camera movement and orientation.
 *
 * A. TRANSLATIVE ROTATION MATH
 * Arrow keys translate the camera's X and Z coordinates based on the current yaw rotation angle:
 *
 *     posX += sin(angle) * speed
 *     posZ -= cos(angle) * speed
 *
 * This provides natural, intuitive first-person controls.
 */
"""

    # Slices mapping (0-indexed line ranges from original_Source_utf8.cpp)
    slices = [
        (0, 35, header),
        (35, 95, solidbox_theory),
        (95, 128, drawtree_theory),
        (128, 156, cobblestone_theory),
        (156, 176, bush_theory),
        (176, 188, grass_theory),
        (188, 205, ditch_theory),
        (205, 222, ditchhoriz_theory),
        (222, 287, door_theory),
        (287, 441, student_theory),
        (441, 481, students_theory),
        (481, 595, classroom_theory),
        (595, 623, stars_theory),
        (623, 644, moon_theory),
        (644, 693, sun_theory),
        (693, 721, cloud_theory),
        (721, 745, sky_theory),
        (745, 803, car_theory),
        (803, 924, earth_theory),
        (924, 1084, staircase_theory),
        (1084, 1462, building_theory),
        (1462, 1526, display_theory),
        (1526, 1550, skybox_theory),
        (1550, 1591, init_theory),
        (1591, 1601, reshape_theory),
        (1601, 1638, keyboard_theory),
        (1638, 1771, specialkeys_theory)
    ]

    print("Generating beautifully documented Source.cpp...")
    output_lines = []

    for start, end, theory_text in slices:
        # Write theory comment block
        output_lines.append(theory_text + "\n")
        # Write corresponding code lines from original source code
        for idx in range(start, end):
            if idx < len(lines):
                output_lines.append(lines[idx])

    with open(output_path, "w", encoding="utf-8") as f:
        f.writelines(output_lines)

    # Let's count lines of the generated file
    with open(output_path, "r", encoding="utf-8") as f:
        gen_lines = f.readlines()

    print(f"Generation successful! New Source.cpp has {len(gen_lines)} lines.")

if __name__ == "__main__":
    main()
