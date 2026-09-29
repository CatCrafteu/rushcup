import { Metadata } from 'next';
import Link from 'next/link';

export const metadata: Metadata = {
  title: 'Terms of Service - CS2 Boost',
  description: 'Terms of Service for CS2 Boost professional boosting services.',
};

export default function TermsPage() {
  const lastUpdated = 'September 22, 2026';
  
  return (
    <div className="min-h-screen py-16 px-4">
      <div className="container max-w-3xl">
        <header className="mb-12 text-center">
          <Link href="/" className="font-bold text-2xl inline-block mb-8">CS2 Boost</Link>
          <h1 className="text-4xl font-bold mb-4">Terms of Service</h1>
          <p className="text-[var(--muted-foreground)]">Last updated: {lastUpdated}</p>
        </header>
        
        <article className="prose prose-invert max-w-none space-y-8">
          <section>
            <h2 className="text-2xl font-bold">1. Acceptance of Terms</h2>
            <p>
              By accessing or using CS2 Boost ("the Service"), you agree to be bound by these Terms of Service ("Terms"). 
              If you disagree with any part of these Terms, you may not use the Service.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">2. Description of Service</h2>
            <p>
              CS2 Boost provides professional Counter-Strike 2 ranking services ("Boosting"). 
              Customers provide their Steam account credentials to verified boosters who play on their accounts 
              to achieve desired ranks in Wingman, Premier, Competitive, or Faceit modes.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">3. User Accounts</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>You must be at least 18 years old to use this Service.</li>
              <li>You are responsible for maintaining the confidentiality of your account credentials.</li>
              <li>You must provide accurate and complete registration information.</li>
              <li>You may not transfer or sell your account to third parties.</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">4. Orders and Payments</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>All prices are displayed in USD equivalent. Payment is processed via cryptocurrency (USDT, BTC, ETH, LTC).</li>
              <li>Orders are created in PENDING_PAYMENT status and require full payment before assignment.</li>
              <li>Payments are processed through Coinbase Commerce or NowPayments. We do not store payment details.</li>
              <li>Refunds are available only if the order has not been assigned to a booster.</li>
              <li>Once a booster starts the order, refunds are at our discretion based on progress completed.</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">5. Account Credentials and Security</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>You acknowledge that providing Steam credentials carries inherent risk.</li>
              <li>Credentials are encrypted at rest using AES-256-GCM and only accessible to assigned boosters.</li>
              <li>We do not guarantee against Valve Anti-Cheat (VAC) bans, game bans, or account restrictions.</li>
              <li>Steam Guard codes are required for login. You must provide them promptly when requested.</li>
              <li>We recommend enabling Steam Mobile Authenticator for additional security.</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">6. Booster Conduct</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>Boosters must not use cheats, hacks, or unauthorized third-party software.</li>
              <li>Boosters must not engage in toxic behavior, griefing, or communication with other players.</li>
              <li>Boosters must complete orders within the estimated timeframe or communicate delays.</li>
              <li>Violations result in immediate account suspension and potential legal action.</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">7. Disputes and Refunds</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>Disputes must be opened within 48 hours of order completion.</li>
              <li>Evidence (screenshots, videos) is required for dispute resolution.</li>
              <li>Partial refunds may be issued for incomplete progress.</li>
              <li>Full refunds for orders not started within 24 hours of payment.</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">8. Limitation of Liability</h2>
            <p>
              CS2 Boost is not liable for any direct, indirect, incidental, or consequential damages resulting from 
              the use of our Service, including but not limited to: account bans, loss of items, rank loss, 
              or emotional distress. Maximum liability is limited to the amount paid for the specific order.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">9. Intellectual Property</h2>
            <p>
              All content, branding, and technology on this Service are the property of CS2 Boost. 
              You may not reproduce, distribute, or create derivative works without written permission.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">10. Termination</h2>
            <p>
              We may terminate or suspend your account immediately for violations of these Terms, 
              including fraud, chargebacks, abuse of boosters, or illegal activities.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">11. Governing Law</h2>
            <p>
              These Terms are governed by the laws of the jurisdiction in which CS2 Boost operates, 
              without regard to conflict of law principles.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">12. Changes to Terms</h2>
            <p>
              We reserve the right to modify these Terms at any time. Continued use of the Service 
              after changes constitutes acceptance. Material changes will be communicated via email.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">13. Contact</h2>
            <p>
              For questions about these Terms, contact us through the dashboard messaging system 
              or email support@cs2boost.com.
            </p>
          </section>
        </article>
        
        <footer className="mt-12 text-center">
          <Link href="/" className="text-[var(--primary)] hover:underline">← Back to Home</Link>
          <span className="mx-2">|</span>
          <Link href="/rushcup" className="text-[var(--primary)] hover:underline">RUSH CUP</Link>
        </footer>
      </div>
    </div>
  );
}