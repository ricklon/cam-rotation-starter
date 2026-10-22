import { defineConfig } from 'vite';

// During `npm run dev`, API and WebSocket calls go to a real CamBox.
// Point at yours with:  CAMBOX=http://192.168.4.1 npm run dev
const device = process.env.CAMBOX || 'http://cambox.local';

export default defineConfig({
  base: './',
  build: { outDir: 'dist', emptyOutDir: true, assetsInlineLimit: 0, target: 'es2019' },
  server: {
    proxy: {
      '/api': device,
      '/ws': { target: device.replace(/^http/, 'ws'), ws: true },
    },
  },
});
