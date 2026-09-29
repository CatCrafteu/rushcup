const requiredEnvVars = [
  'DATABASE_URL',
  'NEXT_PUBLIC_APP_URL',
  'APP_SECRET',
  'JWT_SECRET',
  'ENCRYPTION_KEY',
  'ENCRYPTION_IV',
] as const;

const optionalEnvVars = [
  'NODE_ENV',
  'JWT_EXPIRES_IN',
  'REFRESH_TOKEN_EXPIRES_IN',
  'BCRYPT_ROUNDS',
  'RATE_LIMIT_WINDOW_MS',
  'RATE_LIMIT_MAX_REQUESTS',
  'AUTH_RATE_LIMIT_MAX',
  'API_RATE_LIMIT_MAX',
  'CSP_ENABLED',
  'CSRF_ENABLED',
  'SECURE_COOKIES',
  'SAME_SITE_COOKIE',
  'ALLOWED_ORIGINS',
  'COINBASE_COMMERCE_API_KEY',
  'COINBASE_COMMERCE_WEBHOOK_SECRET',
  'COINBASE_COMMERCE_WEBHOOK_URL',
  'NOWPAYMENTS_API_KEY',
  'NOWPAYMENTS_IPN_SECRET',
  'NOWPAYMENTS_IPN_URL',
  'STEAM_API_KEY',
  'ADMIN_EMAILS',
  'SMTP_HOST',
  'SMTP_PORT',
  'SMTP_USER',
  'SMTP_PASS',
  'EMAIL_FROM',
  'SENTRY_DSN',
  'LOG_LEVEL',
] as const;

type RequiredEnvVar = (typeof requiredEnvVars)[number];
type OptionalEnvVar = (typeof optionalEnvVars)[number];

export function validateEnv(): void {
  const missing: string[] = [];
  const weak: string[] = [];

  for (const key of requiredEnvVars) {
    const value = process.env[key];
    if (!value || value.trim() === '') {
      missing.push(key);
    } else if (key.includes('SECRET') || key.includes('KEY') || key.includes('PASSWORD')) {
      if (value.length < 32) {
        weak.push(`${key} (too short, min 32 chars)`);
      }
      if (value.includes('dev-') || value.includes('change-in-production')) {
        weak.push(`${key} (using default dev value)`);
      }
    }
  }

  if (missing.length > 0) {
    throw new Error(`Missing required environment variables: ${missing.join(', ')}`);
  }

  if (weak.length > 0 && process.env.NODE_ENV === 'production') {
    console.warn('[SECURITY WARNING] Weak environment variables detected:', weak.join(', '));
  }

  if (process.env.NODE_ENV === 'production') {
    if (process.env.SECURE_COOKIES !== 'true') {
      console.warn('[SECURITY WARNING] SECURE_COOKIES should be true in production');
    }
    if (process.env.CSP_ENABLED !== 'true') {
      console.warn('[SECURITY WARNING] CSP_ENABLED should be true in production');
    }
    if (process.env.CSRF_ENABLED !== 'true') {
      console.warn('[SECURITY WARNING] CSRF_ENABLED should be true in production');
    }
  }

  console.log('[ENV] Environment validation passed');
}

export function getEnv<T extends string>(key: T, defaultValue?: string): string {
  const value = process.env[key];
  if (value === undefined) {
    if (defaultValue !== undefined) return defaultValue;
    throw new Error(`Environment variable ${key} is not set`);
  }
  return value;
}

export function getEnvBool(key: string, defaultValue = false): boolean {
  const value = process.env[key];
  if (value === undefined) return defaultValue;
  return value.toLowerCase() === 'true';
}

export function getEnvNumber(key: string, defaultValue: number): number {
  const value = process.env[key];
  if (value === undefined) return defaultValue;
  const parsed = Number(value);
  if (Number.isNaN(parsed)) return defaultValue;
  return parsed;
}

export const env = {
  databaseUrl: getEnv('DATABASE_URL'),
  appUrl: getEnv('NEXT_PUBLIC_APP_URL'),
  appSecret: getEnv('APP_SECRET'),
  jwtSecret: getEnv('JWT_SECRET'),
  jwtExpiresIn: getEnv('JWT_EXPIRES_IN', '7d'),
  refreshTokenExpiresIn: getEnv('REFRESH_TOKEN_EXPIRES_IN', '30d'),
  bcryptRounds: getEnvNumber('BCRYPT_ROUNDS', 12),
  encryptionKey: getEnv('ENCRYPTION_KEY'),
  encryptionIv: getEnv('ENCRYPTION_IV'),
  rateLimit: {
    windowMs: getEnvNumber('RATE_LIMIT_WINDOW_MS', 900000),
    maxRequests: getEnvNumber('RATE_LIMIT_MAX_REQUESTS', 100),
    authMax: getEnvNumber('AUTH_RATE_LIMIT_MAX', 5),
    apiMax: getEnvNumber('API_RATE_LIMIT_MAX', 60),
  },
  security: {
    cspEnabled: getEnvBool('CSP_ENABLED', true),
    csrfEnabled: getEnvBool('CSRF_ENABLED', true),
    secureCookies: getEnvBool('SECURE_COOKIES', process.env.NODE_ENV === 'production'),
    sameSiteCookie: (getEnv('SAME_SITE_COOKIE', 'lax') as 'strict' | 'lax' | 'none'),
  },
  cors: {
    allowedOrigins: getEnv('ALLOWED_ORIGINS', 'http://localhost:3000').split(',').map(s => s.trim()),
  },
  payments: {
    coinbase: {
      apiKey: getEnv('COINBASE_COMMERCE_API_KEY', ''),
      webhookSecret: getEnv('COINBASE_COMMERCE_WEBHOOK_SECRET', ''),
      webhookUrl: getEnv('COINBASE_COMMERCE_WEBHOOK_URL', ''),
    },
    nowpayments: {
      apiKey: getEnv('NOWPAYMENTS_API_KEY', ''),
      ipnSecret: getEnv('NOWPAYMENTS_IPN_SECRET', ''),
      ipnUrl: getEnv('NOWPAYMENTS_IPN_URL', ''),
    },
  },
  steam: {
    apiKey: getEnv('STEAM_API_KEY', ''),
  },
  admin: {
    emails: getEnv('ADMIN_EMAILS', '').split(',').map(s => s.trim().toLowerCase()),
  },
  email: {
    host: getEnv('SMTP_HOST', ''),
    port: getEnvNumber('SMTP_PORT', 587),
    user: getEnv('SMTP_USER', ''),
    pass: getEnv('SMTP_PASS', ''),
    from: getEnv('EMAIL_FROM', ''),
  },
  monitoring: {
    sentryDsn: getEnv('SENTRY_DSN', ''),
    logLevel: getEnv('LOG_LEVEL', 'info'),
  },
  nodeEnv: getEnv('NODE_ENV', 'development'),
};