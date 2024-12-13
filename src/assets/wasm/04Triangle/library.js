mergeInto(LibraryManager.library, {
	
    WAJS_SetupCanvas: function(width, height) {
		this.initTime = Date.now();

        const canvas = document.getElementById("canvas");
		canvas.getContext("webgl")
        canvas.width = width;
        canvas.height = height;
		
		var attr = { majorVersion: 1, minorVersion: 0, antialias: true, alpha: false };
		var errorInfo = '';
		try
		{
			let onContextCreationError = function(event) { errorInfo = event.statusMessage || errorInfo; };
			canvas.addEventListener('webglcontextcreationerror', onContextCreationError, false);
			try { GLctx = canvas.getContext('webgl', attr) || canvas.getContext('experimental-webgl', attr); }
			finally { canvas.removeEventListener('webglcontextcreationerror', onContextCreationError, false); }
			if (!GLctx) throw 'Could not create context';
		}
		catch (e) { abort('WEBGL', e + (errorInfo ? ' (' + errorInfo + ')' : '')); }

		var exts = GLctx.getSupportedExtensions();
		if (exts && exts.length > 0)
		{
			// These are the 'safe' feature-enabling extensions that don't add any performance impact related to e.g. debugging, and
			// should be enabled by default so that client GLES2/GL code will not need to go through extra hoops to get its stuff working.
			// As new extensions are ratified at http://www.khronos.org/registry/webgl/extensions/ , feel free to add your new extensions
			// here, as long as they don't produce a performance impact for users that might not be using those extensions.
			// E.g. debugging-related extensions should probably be off by default.
			var W = 'WEBGL_', O = 'OES_', E = 'EXT_', T = 'texture_', C = 'compressed_'+T;
			var automaticallyEnabledExtensions = [ // Khronos ratified WebGL extensions ordered by number (no debug extensions):
				O+T+'float', O+T+'half_float', O+'standard_derivatives',
				O+'vertex_array_object', W+C+'s3tc', W+'depth_texture',
				O+'element_index_uint', E+T+'filter_anisotropic', E+'frag_depth',
				W+'draw_buffers', 'ANGLE_instanced_arrays', O+T+'float_linear',
				O+T+'half_float_linear', E+'blend_minmax', E+'shader_texture_lod',
				// Community approved WebGL extensions ordered by number:
				W+C+'pvrtc', E+'color_buffer_half_float', W+'color_buffer_float',
				E+'sRGB', W+C+'etc1', E+'disjoint_timer_query',
				W+C+'etc', W+C+'astc', E+'color_buffer_float',
				W+C+'s3tc_srgb', E+'disjoint_timer_query_webgl2'];
			exts.forEach(function(ext)
			{
				if (automaticallyEnabledExtensions.indexOf(ext) != -1)
				{
					// Calling .getExtension enables that extension permanently, no need to store the return value to be enabled.
					GLctx.getExtension(ext);
				}
			});
		}

		var draw_func_ex = function() { if (ABORT) return; window.requestAnimationFrame(draw_func_ex); wasmExports.WAFNDraw(); };
		window.requestAnimationFrame(draw_func_ex);
		
		
    },
	WAJS_GetTime: function() {
         return Date.now() - this.initTime; 
    }
});

