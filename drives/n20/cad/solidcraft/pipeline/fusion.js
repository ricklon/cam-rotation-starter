// node fusion.js <script.py> : runs a Fusion Python script (def run(_context)) via Fusion's MCP HTTP server
const fs = require('fs');
const URL = 'http://127.0.0.1:27182/mcp';
let sid = null;
async function rpc(method, params, id) {
  const h = { 'Content-Type': 'application/json', 'Accept': 'application/json, text/event-stream', 'MCP-Protocol-Version': '2025-06-18' };
  if (sid) h['MCP-Session-Id'] = sid;
  const body = { jsonrpc: '2.0', method, params };
  if (id !== undefined) body.id = id;
  const r = await fetch(URL, { method: 'POST', headers: h, body: JSON.stringify(body) });
  if (r.headers.get('mcp-session-id')) sid = r.headers.get('mcp-session-id');
  const t = await r.text();
  if (id === undefined) return null;
  const m = t.match(/^data: (.*)$/m);
  return JSON.parse(m ? m[1] : t);
}
(async () => {
  await rpc('initialize', { protocolVersion: '2025-06-18', capabilities: {}, clientInfo: { name: 'cli', version: '1' } }, 1);
  await rpc('notifications/initialized', {});
  const args = JSON.parse(process.argv[3] || 'null') || { featureType: 'script', object: { script: fs.readFileSync(process.argv[2], 'utf8') } };
  const res = await rpc('tools/call', { name: process.argv[3] ? process.argv[2] : 'fusion_mcp_execute', arguments: args }, 2);
  const c = res.result ? res.result.content : null;
  if (c) for (const x of c) console.log(x.text); else console.log(JSON.stringify(res, null, 1));
})().catch(e => { console.error(e); process.exit(1); });
