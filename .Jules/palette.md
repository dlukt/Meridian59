## 2024-05-24 - Tooltip Pattern Discovery
**Learning:** The client application uses a custom `TooltipAddWindow` helper for adding tooltips to standard Win32 controls, rather than standard Windows API calls directly.
**Action:** Use `TooltipAddWindow(control_handle, hInst, string_resource_id)` for any future tooltip additions to ensure consistency and correct behavior.
