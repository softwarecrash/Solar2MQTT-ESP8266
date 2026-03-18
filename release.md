Release Notes

- Web UI now loads full HTML pages without server-side placeholders; dynamic data comes from `/meta` and `/config`.
- Debug page adds a serial loopback test and a downloadable debug report for current raw/parsed data.
- Favicon is served as `favicon.ico` and the GitHub update check is cached to reduce browser/heap load.
- WebSocket handling and page streaming were optimized to lower heap usage and reduce random page load failures.
- Protocol parsing was expanded to better handle variable inverter response lengths.
