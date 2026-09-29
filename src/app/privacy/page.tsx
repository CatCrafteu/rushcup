import { Metadata } from 'next';
import Link from 'next/link';

export const metadata: Metadata = {
  title: 'Privacy Policy - CS2 Boost',
  description: 'Privacy Policy for CS2 Boost professional boosting services.',
};

export default function PrivacyPage() {
  const lastUpdated = 'September 22, 2026';
  
  return (
    <div className="min-h-screen py-16 px-4">
      <div className="container max-w-3xl">
        <header className="mb-12 text-center">
          <Link href="/" className="font-bold text-2xl inline-block mb-8">CS2 Boost</Link>
          <h1 className="text-4xl font-bold mb-4">Privacy Policy</h1>
          <p className="text-[var(--muted-foreground)]">Last updated: {lastUpdated}</p>
        </header>
        
        <article className="prose prose-invert max-w-none space-y-8">
          <section>
            <h2 className="text-2xl font-bold">1. Information We Collect</h2>
            <h3 className="text-xl font-semibold mt-4">Account Information</h3>
            <ul className="list-disc pl-6 space-y-2">
              <li>Email address (for authentication and notifications)</li>
              <li>Password hash (bcrypt, never stored in plaintext)</li>
              <li>Role (Customer, Booster, Admin)</li>
              <li>Steam ID, Steam name, Steam avatar (optional, for boosters)</li>
            </ul>
            
            <h3 className="text-xl font-semibold mt-4">Order Information</h3>
            <ul className="list-disc pl-6 space-y-2">
              <li>Current and target ranks</li>
              <li>Steam login credentials (encrypted with AES-256-GCM)</li>
              <li>Steam Guard codes (encrypted, temporary)</li>
              <li>Order status, progress logs, chat messages</li>
            </ul>
            
            <h3 className="text-xl font-semibold mt-4">Payment Information</h3>
            <ul className="list-disc pl-6 space-y-2">
              <li>Cryptocurrency transaction IDs (Coinbase Commerce / NowPayments)</li>
              <li>Payment amounts, currencies, statuses</li>
              <li>We do NOT store wallet addresses, private keys, or payment credentials</li>
            </ul>
            
            <h3 className="text-xl font-semibold mt-4">Technical Data</h3>
            <ul className="list-disc pl-6 space-y-2">
              <li>IP addresses (for rate limiting and security)</li>
              <li>User agent strings</li>
              <li>Session tokens (hashed)</li>
              <li>Audit logs of administrative actions</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">2. How We Use Your Information</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>Provide and maintain the boosting service</li>
              <li>Authenticate and authorize user access</li>
              <li>Process orders and match with boosters</li>
              <li>Facilitate communication between customers and boosters</li>
              <li>Process cryptocurrency payments via third-party providers</li>
              <li>Send service-related notifications (order updates, payment confirmations)</li>
              <li>Prevent fraud, abuse, and security incidents</li>
              <li>Comply with legal obligations</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">3. Data Sharing and Disclosure</h2>
            <p>We do not sell your personal information. We share data only in these circumstances:</p>
            <ul className="list-disc pl-6 space-y-2">
              <li><strong>Assigned Boosters:</strong> Steam credentials (decrypted) are shared ONLY with the booster assigned to your order.</li>
              <li><strong>Payment Processors:</strong> Transaction data shared with Coinbase Commerce / NowPayments for payment processing.</li>
              <li><strong>Legal Requirements:</strong> When required by law, court order, or government request.</li>
              <li><strong>Security:</strong> To prevent fraud, investigate abuse, or protect rights and safety.</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">4. Data Security</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>Steam credentials encrypted at rest with AES-256-GCM (unique salt per record)</li>
              <li>Passwords hashed with bcrypt (cost factor 12)</li>
              <li>JWT tokens for authentication (HS256, 7-day access, 30-day refresh)</li>
              <li>Secure, HttpOnly, SameSite cookies</li>
              <li>Rate limiting on all endpoints</li>
              <li>Content Security Policy headers</li>
              <li>CORS restrictions to known origins</li>
              <li>Regular dependency auditing</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">5. Data Retention</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li>Account data: Retained while account is active</li>
              <li>Order data: Retained for 2 years after completion for dispute resolution</li>
              <li>Payment data: Retained for 5 years for financial compliance</li>
              <li>Audit logs: Retained for 1 year</li>
              <li>Session data: Expired tokens purged automatically</li>
              <li>Deleted accounts: Data anonymized within 30 days</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">6. Your Rights</h2>
            <p>Depending on your jurisdiction, you may have the right to:</p>
            <ul className="list-disc pl-6 space-y-2">
              <li>Access your personal data</li>
              <li>Rectify inaccurate data</li>
              <li>Erase your data (right to be forgotten)</li>
              <li>Restrict processing</li>
              <li>Data portability</li>
              <li>Object to processing</li>
              <li>Withdraw consent (where applicable)</li>
            </ul>
            <p>To exercise these rights, contact us through the dashboard or email privacy@cs2boost.com.</p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">7. Third-Party Services</h2>
            <ul className="list-disc pl-6 space-y-2">
              <li><strong>Coinbase Commerce:</strong> Cryptocurrency payment processing. <a href="https://commerce.coinbase.com/privacy" target="_blank" rel="noopener noreferrer" className="text-[var(--primary)]">Privacy Policy</a></li>
              <li><strong>NowPayments:</strong> Cryptocurrency payment processing. <a href="https://nowpayments.io/privacy-policy" target="_blank" rel="noopener noreferrer" className="text-[var(--primary)]">Privacy Policy</a></li>
              <li><strong>Steam API:</strong> Public profile data retrieval. <a href="https://store.steampowered.com/privacy_agreement/" target="_blank" rel="noopener noreferrer" className="text-[var(--primary)]">Privacy Policy</a></li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">8. Cookies and Tracking</h2>
            <p>We use only essential cookies:</p>
            <ul className="list-disc pl-6 space-y-2">
              <li>Session cookies (authentication)</li>
              <li>CSRF protection cookies</li>
              <li>No analytics, advertising, or tracking cookies</li>
              <li>No third-party cookies</li>
            </ul>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">9. International Transfers</h2>
            <p>
              Data may be processed in countries with different data protection laws. 
              We ensure appropriate safeguards (standard contractual clauses) for transfers.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">10. Children's Privacy</h2>
            <p>
              Our Service is not intended for users under 18. We do not knowingly collect 
              data from minors. If you believe a minor has provided data, contact us immediately.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">11. Changes to This Policy</h2>
            <p>
              We may update this Privacy Policy. Material changes will be communicated via email 
              and dashboard notification. Continued use constitutes acceptance.
            </p>
          </section>
          
          <section>
            <h2 className="text-2xl font-bold">12. Contact</h2>
            <p>
              For privacy concerns or data requests, contact: privacy@cs2boost.com 
              or use the dashboard messaging system.
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