import DOMPurify from 'isomorphic-dompurify';
import { z } from 'zod';

export const sanitizeHtml = (dirty: string): string => {
  return DOMPurify.sanitize(dirty, {
    ALLOWED_TAGS: ['b', 'i', 'em', 'strong', 'p', 'br', 'ul', 'ol', 'li', 'a', 'code', 'pre', 'blockquote'],
    ALLOWED_ATTR: ['href', 'target', 'rel', 'class'],
    ALLOW_DATA_ATTR: false,
    FORBID_TAGS: ['script', 'style', 'iframe', 'object', 'embed', 'form', 'input', 'button'],
    FORBID_ATTR: ['onerror', 'onload', 'onclick', 'onmouseover', 'onfocus', 'onblur'],
  });
};

export const sanitizeText = (dirty: string): string => {
  return DOMPurify.sanitize(dirty, {
    ALLOWED_TAGS: [],
    ALLOWED_ATTR: [],
  }).replace(/\s+/g, ' ').trim();
};

export const sanitizeForLog = (input: string): string => {
  return input
    .replace(/[\r\n\t]/g, ' ')
    .replace(/[<>]/g, '')
    .substring(0, 1000);
};

export const steamCredentialSchema = z.object({
  login: z.string().min(3).max(64).regex(/^[a-zA-Z0-9._-]+$/),
  password: z.string().min(8).max(128),
  guardCode: z.string().regex(/^[A-Z0-9]{5}$/).optional(),
});

export const orderSchema = z.object({
  boostType: z.enum(['WINGMAN', 'PREMIER', 'COMPETITIVE', 'FACEIT']),
  currentRank: z.string().min(1).max(32),
  targetRank: z.string().min(1).max(32),
  currentElo: z.number().int().min(0).max(50000).optional(),
  targetElo: z.number().int().min(0).max(50000).optional(),
  notes: z.string().max(1000).optional(),
  steamLogin: z.string().min(3).max(64).regex(/^[a-zA-Z0-9._-]+$/),
  steamPassword: z.string().min(8).max(128),
  steamGuardCode: z.string().regex(/^[A-Z0-9]{5}$/).optional(),
});

export const loginSchema = z.object({
  email: z.string().email().max(254),
  password: z.string().min(8).max(128),
});

export const registerSchema = loginSchema.extend({
  role: z.enum(['USER', 'BOOSTER']).default('USER'),
});

export const messageSchema = z.object({
  message: z.string().min(1).max(5000),
});

export function validateInput<T>(schema: z.ZodSchema<T>, data: unknown): { success: true; data: T } | { success: false; errors: string[] } {
  const result = schema.safeParse(data);
  if (result.success) {
    return { success: true, data: result.data };
  }
  return {
    success: false,
    errors: result.error.issues.map(i => `${i.path.join('.')}: ${i.message}`),
  };
}

export const cspDirectives = {
  'default-src': ["'self'"],
  'script-src': ["'self'", "'unsafe-inline'", "'unsafe-eval'"],
  'style-src': ["'self'", "'unsafe-inline'"],
  'img-src': ["'self'", 'data:', 'https:', 'blob:'],
  'font-src': ["'self'", 'data:'],
  'connect-src': ["'self'", 'https://api.coinbase.com', 'https://api.nowpayments.io'],
  'frame-src': ["'none'"],
  'object-src': ["'none'"],
  'base-uri': ["'self'"],
  'form-action': ["'self'"],
  'frame-ancestors': ["'none'"],
  'upgrade-insecure-requests': [],
};

export function buildCspHeader(nonce?: string): string {
  const directives = { ...cspDirectives };
  
  if (nonce) {
    directives['script-src'] = ["'self'", `'nonce-${nonce}'`, "'unsafe-eval'"];
    directives['style-src'] = ["'self'", `'nonce-${nonce}'`];
  }
  
  return Object.entries(directives)
    .map(([key, values]) => values.length ? `${key} ${values.join(' ')}` : key)
    .join('; ');
}