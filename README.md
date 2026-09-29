# CS2 Boost Platform

Professional CS2 boosting platform built with Next.js 14, Prisma, and TypeScript.

## Features

- **Authentication**: JWT-based auth with secure HttpOnly cookies, role-based access (User/Booster/Admin)
- **Orders**: Full order lifecycle (create, pay, assign, track, complete)
- **Payments**: Crypto payments via Coinbase Commerce & NowPayments (USDT, BTC, ETH)
- **Security**: Rate limiting, CSP, CORS, XSS sanitization, encrypted credentials (AES-256-GCM)
- **Booster Panel**: Claim orders, track progress, manage profile
- **Admin Panel**: User management, order oversight, payments, audit logs
- **Real-time Chat**: Customer-Booster communication per order

## Tech Stack

- Next.js 14 (App Router)
- TypeScript
- Prisma ORM (SQLite/Turso)
- Tailwind CSS
- jose (JWT)
- rate-limiter-flexible
- isomorphic-dompurify (XSS protection)

## Getting Started

1. **Install dependencies**
   ```bash
   npm install
   ```

2. **Configure environment**
   ```bash
   cp .env.example .env
   # Edit .env with your values
   ```

3. **Generate Prisma client & push schema**
   ```bash
   npm run db:generate
   npm run db:push
   ```

4. **Seed database (optional)**
   ```bash
   npm run db:seed
   ```

5. **Run development server**
   ```bash
   npm run dev
   ```

6. **Open http://localhost:3000**

## Environment Variables

See `.env.example` for all required variables. Key ones:

- `DATABASE_URL` - SQLite file path or Turso connection string
- `JWT_SECRET` - 32+ char secret for JWT signing
- `ENCRYPTION_KEY` - 32-byte hex for AES-256-GCM credential encryption
- `ENCRYPTION_IV` - 16-byte hex for encryption IV
- `COINBASE_COMMERCE_API_KEY` - Coinbase Commerce API key
- `COINBASE_COMMERCE_WEBHOOK_SECRET` - Webhook signature secret
- `NOWPAYMENTS_API_KEY` - NowPayments API key
- `NOWPAYMENTS_IPN_SECRET` - IPN signature secret

## Production Deployment

1. Set `NODE_ENV=production`
2. Use strong secrets (32+ chars)
3. Set `SECURE_COOKIES=true`
4. Configure `ALLOWED_ORIGINS` with your domain
5. Set up SSL/TLS
6. Configure webhook URLs for payment providers
7. Run `npm run build && npm start`

## Security

- All credentials encrypted at rest (AES-256-GCM)
- Passwords hashed with bcrypt (cost 12)
- JWT tokens: 7-day access, 30-day refresh
- Rate limiting on all endpoints
- CSP headers on all responses
- CORS restricted to configured origins
- Input validation with Zod
- XSS sanitization on all user inputs
- Audit logging for admin actions

## API Endpoints

### Auth
- `POST /api/auth/register` - Register
- `POST /api/auth/login` - Login
- `POST /api/auth/logout` - Logout
- `POST /api/auth/refresh` - Refresh tokens
- `GET /api/auth/me` - Current user

### Orders
- `GET /api/orders` - List orders
- `POST /api/orders` - Create order
- `GET /api/orders/[id]` - Get order details
- `PATCH /api/orders/[id]` - Update order
- `POST /api/orders/[id]/chat` - Send message

### Boosters
- `GET /api/boosters` - List boosters
- `POST /api/boosters` - Create booster profile
- `GET /api/boosters/[id]` - Get booster
- `PATCH /api/boosters/[id]` - Update profile

### Payments
- `POST /api/payments/create` - Create payment
- `POST /api/payments/webhook/coinbase` - Coinbase webhook
- `POST /api/payments/webhook/nowpayments` - NowPayments webhook

### Admin
- `GET /api/admin?section=overview|users|orders|boosters|payments|audit`
- `PATCH /api/admin/users/[id]` - Update user
- `DELETE /api/admin/users/[id]` - Delete user

### Health
- `GET /api/health` - Health check

## Scripts

- `npm run dev` - Development server
- `npm run build` - Production build
- `npm run start` - Production server
- `npm run lint` - ESLint
- `npm run db:generate` - Generate Prisma client
- `npm run db:push` - Push schema to DB
- `npm run db:studio` - Prisma Studio
- `npm run db:seed` - Seed database
- `npm run security:audit` - npm audit
- `npm run security:check` - Custom security checks

## License

Private - All rights reserved.