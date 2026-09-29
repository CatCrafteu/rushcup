import { execSync } from 'child_process';
import fs from 'fs';
import path from 'path';

interface SecurityIssue {
  type: 'critical' | 'high' | 'medium' | 'low';
  message: string;
  file?: string;
  line?: number;
}

const issues: SecurityIssue[] = [];

function addIssue(type: SecurityIssue['type'], message: string, file?: string, line?: number) {
  issues.push({ type, message, file, line });
}

function checkPackageJson() {
  const pkgPath = path.join(process.cwd(), 'package.json');
  if (!fs.existsSync(pkgPath)) return;
  
  const pkg = JSON.parse(fs.readFileSync(pkgPath, 'utf8'));
  
  if (pkg.dependencies) {
    for (const [name, version] of Object.entries(pkg.dependencies)) {
      if (typeof version === 'string' && version.includes('*')) {
        addIssue('medium', `Dependency ${name} uses wildcard version: ${version}`, 'package.json');
      }
    }
  }
  
  if (pkg.devDependencies) {
    for (const [name, version] of Object.entries(pkg.devDependencies)) {
      if (typeof version === 'string' && version.includes('*')) {
        addIssue('low', `Dev dependency ${name} uses wildcard version: ${version}`, 'package.json');
      }
    }
  }
}

function checkEnvFiles() {
  const envPath = path.join(process.cwd(), '.env');
  const envExamplePath = path.join(process.cwd(), '.env.example');
  
  if (!fs.existsSync(envExamplePath)) {
    addIssue('high', 'Missing .env.example file');
  }
  
  if (fs.existsSync(envPath)) {
    const envContent = fs.readFileSync(envPath, 'utf8');
    const lines = envContent.split('\n');
    
    lines.forEach((line, i) => {
      const trimmed = line.trim();
      if (trimmed.startsWith('#') || !trimmed) return;
      
      const [key, value] = trimmed.split('=');
      if (!value) return;
      
      const cleanValue = value.trim().replace(/^["']|["']$/g, '');
      
      if (key.includes('SECRET') || key.includes('KEY') || key.includes('PASSWORD')) {
        if (cleanValue.length < 32) {
          addIssue('high', `Weak secret: ${key} (${cleanValue.length} chars, min 32)`, '.env', i + 1);
        }
        if (cleanValue.includes('dev-') || cleanValue.includes('change-in-production') || cleanValue.includes('example')) {
          addIssue('critical', `Production secret using dev value: ${key}`, '.env', i + 1);
        }
      }
      
      if (key === 'NODE_ENV' && cleanValue === 'development') {
        addIssue('medium', 'NODE_ENV is set to development', '.env', i + 1);
      }
      
      if (key === 'SECURE_COOKIES' && cleanValue !== 'true') {
        addIssue('high', 'SECURE_COOKIES should be true in production', '.env', i + 1);
      }
    });
  }
}

function checkSourceFiles() {
  const srcDir = path.join(process.cwd(), 'src');
  
  function walk(dir: string) {
    const files = fs.readdirSync(dir);
    
    for (const file of files) {
      const fullPath = path.join(dir, file);
      const stat = fs.statSync(fullPath);
      
      if (stat.isDirectory()) {
        walk(fullPath);
      } else if (file.endsWith('.ts') || file.endsWith('.tsx')) {
        const content = fs.readFileSync(fullPath, 'utf8');
        const lines = content.split('\n');
        
        lines.forEach((line, i) => {
          const trimmed = line.trim();
          
          if (trimmed.includes('console.log') && !trimmed.includes('//')) {
            addIssue('low', 'console.log found in production code', fullPath, i + 1);
          }
          
          if (trimmed.includes('eval(') || trimmed.includes('Function(')) {
            addIssue('critical', 'Dynamic code execution detected (eval/Function)', fullPath, i + 1);
          }
          
          if (trimmed.includes('dangerouslySetInnerHTML')) {
            addIssue('high', 'dangerouslySetInnerHTML usage - XSS risk', fullPath, i + 1);
          }
          
          if (trimmed.match(/innerHTML\s*=/)) {
            addIssue('medium', 'Direct innerHTML assignment - potential XSS', fullPath, i + 1);
          }
          
          if (trimmed.includes('NEXT_PUBLIC_') && (trimmed.includes('SECRET') || trimmed.includes('KEY'))) {
            addIssue('critical', 'Secret exposed via NEXT_PUBLIC_ prefix', fullPath, i + 1);
          }
          
          if (trimmed.includes('process.env.') && trimmed.includes('SECRET') && !trimmed.includes('process.env.JWT_SECRET')) {
            const match = trimmed.match(/process\.env\.([A-Z_]+)/);
            if (match && !['NODE_ENV', 'NEXT_PUBLIC_APP_URL', 'DATABASE_URL'].includes(match[1])) {
              addIssue('medium', `Direct env access in component: ${match[1]}`, fullPath, i + 1);
            }
          }
        });
      }
    }
  }
  
  if (fs.existsSync(srcDir)) {
    walk(srcDir);
  }
}

function checkMiddleware() {
  const middlewarePath = path.join(process.cwd(), 'src', 'middleware.ts');
  const middlewareSrcPath = path.join(process.cwd(), 'src', 'lib', 'api-middleware.ts');
  
  if (!fs.existsSync(middlewarePath) && !fs.existsSync(middlewareSrcPath)) {
    addIssue('high', 'No middleware.ts found for global security headers');
  }
}

function checkAuthImplementation() {
  const authPath = path.join(process.cwd(), 'src', 'lib', 'auth.ts');
  if (fs.existsSync(authPath)) {
    const content = fs.readFileSync(authPath, 'utf8');
    
    if (!content.includes('HttpOnly')) {
      addIssue('high', 'Cookies missing HttpOnly flag');
    }
    if (!content.includes('Secure')) {
      addIssue('high', 'Cookies missing Secure flag');
    }
    if (!content.includes('SameSite')) {
      addIssue('medium', 'Cookies missing SameSite attribute');
    }
    if (!content.includes('bcrypt') && !content.includes('argon2')) {
      addIssue('high', 'Weak password hashing (not bcrypt/argon2)');
    }
  }
}

function checkRateLimiting() {
  const rateLimitPath = path.join(process.cwd(), 'src', 'lib', 'rate-limit.ts');
  if (!fs.existsSync(rateLimitPath)) {
    addIssue('high', 'No rate limiting implementation found');
  }
}

function checkCSP() {
  const securityPath = path.join(process.cwd(), 'src', 'lib', 'security.ts');
  if (fs.existsSync(securityPath)) {
    const content = fs.readFileSync(securityPath, 'utf8');
    if (!content.includes('Content-Security-Policy') && !content.includes('cspDirectives')) {
      addIssue('medium', 'No CSP implementation found');
    }
  }
}

function runAudit() {
  try {
    const result = execSync('npm audit --json', { encoding: 'utf8', stdio: 'pipe' });
    const audit = JSON.parse(result);
    
    if (audit.vulnerabilities) {
      for (const [name, vuln] of Object.entries(audit.vulnerabilities)) {
        const v = vuln as any;
        if (v.severity === 'critical' || v.severity === 'high') {
          addIssue('high', `Vulnerable dependency: ${name} (${v.severity}) - ${v.title || 'No title'}`);
        } else if (v.severity === 'moderate') {
          addIssue('medium', `Vulnerable dependency: ${name} (${v.severity}) - ${v.title || 'No title'}`);
        }
      }
    }
  } catch {
    addIssue('low', 'Could not run npm audit');
  }
}

console.log('🔍 Running security checks...\n');

checkPackageJson();
checkEnvFiles();
checkSourceFiles();
checkMiddleware();
checkAuthImplementation();
checkRateLimiting();
checkCSP();
runAudit();

const critical = issues.filter(i => i.type === 'critical');
const high = issues.filter(i => i.type === 'high');
const medium = issues.filter(i => i.type === 'medium');
const low = issues.filter(i => i.type === 'low');

console.log(`\n📊 Security Check Results:`);
console.log(`  Critical: ${critical.length}`);
console.log(`  High:     ${high.length}`);
console.log(`  Medium:   ${medium.length}`);
console.log(`  Low:      ${low.length}`);

if (issues.length > 0) {
  console.log('\n📋 Issues:');
  
  for (const issue of issues) {
    const prefix = issue.type === 'critical' ? '🔴' : issue.type === 'high' ? '🟠' : issue.type === 'medium' ? '🟡' : '🟢';
    const location = issue.file ? ` (${issue.file}${issue.line ? `:${issue.line}` : ''})` : '';
    console.log(`  ${prefix} [${issue.type.toUpperCase()}] ${issue.message}${location}`);
  }
}

if (critical.length > 0 || high.length > 0) {
  console.log('\n❌ Security check FAILED - Critical/High issues found');
  process.exit(1);
} else if (medium.length > 0) {
  console.log('\n⚠️ Security check PASSED with warnings');
  process.exit(0);
} else {
  console.log('\n✅ Security check PASSED');
  process.exit(0);
}