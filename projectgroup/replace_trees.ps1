$path = "Source.cpp"
$content = [System.IO.File]::ReadAllText($path)

$target_crlf = "    // Background forest line`r`n    for (float x = -40.0f; x <= 40.0f; x += 10.0f) {`r`n        drawTree(x, -12.0f, 1);`r`n    }"
$target_lf = "    // Background forest line`n    for (float x = -40.0f; x <= 40.0f; x += 10.0f) {`n        drawTree(x, -12.0f, 1);`n    }"

$replacement = @"
    // --- Rich Population of Campus with Normal Trees ---
    // This dynamically fills all empty green areas, perfectly avoiding walkways, buildings, roads, and ditches.
    for (float x = -95.0f; x <= 95.0f; x += 10.0f) {
        for (float z = -90.0f; z <= 8.0f; z += 10.0f) {
            // Safety margins around paths
            if ((x >= -27.5f && x <= -21.0f) || (x >= -10.5f && x <= -4.5f) || (x >= 6.25f && x <= 12.25f)) continue;

            // Safety margins around buildings and their hedges
            if (z >= 2.0f && z <= 9.2f) {
                if (x >= -7.0f && x <= 5.2f) continue;      // Center building
                if (x >= -22.0f && x <= -9.8f) continue;    // Left building
            }

            // Safety margins around ditches
            if ((z >= 1.6f && z <= 2.6f) || (z >= 8.6f && z <= 10.4f)) continue;

            // Seed based deterministic random jitter and tree type selection
            int ix = (int)((x + 100.0f) * 7.0f);
            int iz = (int)((z + 100.0f) * 13.0f);
            int seed = ix * 31 + iz * 17;
            
            float ox = (float)(seed % 7) - 3.5f;
            float oz = (float)((seed / 7) % 7) - 3.5f;
            float tx = x + ox;
            float tz = z + oz;
            
            // Re-validate post-jitter bounds to prevent clips
            if (tx < -98.0f || tx > 98.0f || tz < -98.0f || tz > 9.0f) continue;
            if ((tx >= -27.5f && tx <= -21.0f) || (tx >= -10.5f && tx <= -4.5f) || (tx >= 6.25f && tx <= 12.25f)) continue;
            if (tz >= 2.0f && tz <= 9.2f) {
                if (tx >= -7.0f && tx <= 5.2f) continue;
                if (tx >= -22.0f && tx <= -9.8f) continue;
            }
            if ((tz >= 1.6f && tz <= 2.6f) || (tz >= 8.6f && tz <= 10.4f)) continue;

            // Draw deterministic mix of simple evergreen (0) and leafy (1) trees
            int treeType = seed % 2;
            drawTree(tx, tz, treeType);
        }
    }
"@

if ($content.Contains($target_crlf)) {
    $content = $content.Replace($target_crlf, $replacement)
    [System.IO.File]::WriteAllText($path, $content)
    Write-Output "SUCCESS_CRLF"
} elseif ($content.Contains($target_lf)) {
    $content = $content.Replace($target_lf, $replacement)
    [System.IO.File]::WriteAllText($path, $content)
    Write-Output "SUCCESS_LF"
} else {
    Write-Output "ERROR: Target not found"
}
