# ============================================================================
# M130 GCS — Custom Build Overrides
# ============================================================================

# Application Branding
set(QGC_APP_NAME "M130GCS" CACHE STRING "App Name" FORCE)
set(QGC_ORG_NAME "M130 Project" CACHE STRING "Org Name" FORCE)
set(QGC_PACKAGE_NAME "com.m130.gcs" CACHE STRING "Package Name" FORCE)

# Disable APM — صاروخ PX4 فقط
set(QGC_DISABLE_APM_MAVLINK         ON CACHE BOOL "Disable APM Dialect"        FORCE)
set(QGC_DISABLE_APM_PLUGIN          ON CACHE BOOL "Disable APM Plugin"         FORCE)
set(QGC_DISABLE_APM_PLUGIN_FACTORY  ON CACHE BOOL "Disable APM Plugin Factory" FORCE)

# نستخدم مصنع firmware مخصص
set(QGC_DISABLE_PX4_PLUGIN_FACTORY ON CACHE BOOL "Disable PX4 Plugin Factory" FORCE)

# Linux AppImage Icon — the canonical location is custom/res/Images/.
# custom/res/icons/ is kept as a symlink-friendly alias so both paths work.
if(EXISTS "${CMAKE_SOURCE_DIR}/custom/res/Images/rocket_icon.svg")
    set(QGC_APPIMAGE_ICON_SCALABLE_PATH
        "${CMAKE_SOURCE_DIR}/custom/res/Images/rocket_icon.svg"
        CACHE FILEPATH "AppImage Icon SVG Path" FORCE)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/custom/res/icons/rocket_icon.svg")
    set(QGC_APPIMAGE_ICON_SCALABLE_PATH
        "${CMAKE_SOURCE_DIR}/custom/res/icons/rocket_icon.svg"
        CACHE FILEPATH "AppImage Icon SVG Path" FORCE)
endif()
