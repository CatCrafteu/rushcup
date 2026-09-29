import Link from 'next/link';
import { Metadata } from 'next';

export const metadata: Metadata = {
  title: 'CS2 Boost - Professional Ranking Service',
  description: 'Professional CS2 boosting services. Global Elite Wingman, 20k+ Premier. Secure, fast, reliable.',
};

const features = [
  {
    title: 'Global Elite Wingman',
    description: 'Reach the highest rank in Wingman mode with our verified boosters.',
    icon: (
      <svg className="w-8 h-8" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="2">
        <path strokeLinecap="round" strokeLinejoin="round" d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" />
      </svg>
    ),
  },
  {
    title: '20k+ Premier Rating',
    description: 'Push your Premier rating beyond 20,000 with guaranteed results.',
    icon: (
      <svg className="w-8 h-8" fill="currentColor" viewBox="0 0 24 24">
        <path d="M12 2l3.09 6.26L22 9.27l-5 4.87 1.18 6.88L12 17.77l-6.18 3.25L7 14.14 2 9.27l6.91-1.01L12 2z" />
      </svg>
    ),
  },
  {
    title: 'Verified Boosters',
    description: 'All boosters are identity-verified with proven track records.',
    icon: (
      <svg className="w-8 h-8" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="2">
        <path strokeLinecap="round" strokeLinejoin="round" d="M9 12l2 2 4-4m5.618-4.016A11.955 11.955 0 0112 2.944a11.955 11.955 0 01-8.618 3.04A12.02 12.02 0 003 9c0 5.591 3.824 10.29 9 11.622 5.176-1.332 9-6.03 9-11.622 0-1.042-.133-2.052-.382-3.016z" />
      </svg>
    ),
  },
  {
    title: 'Crypto Payments',
    description: 'Pay with USDT, BTC, ETH via Coinbase Commerce or NowPayments.',
    icon: (
      <svg className="w-8 h-8" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="2">
        <path strokeLinecap="round" strokeLinejoin="round" d="M12 8c-1.657 0-3 .895-3 2s1.343 2 3 2 3 .895 3 2-1.343 2-3 2m0-8c1.11 0 2.08.402 2.599 1M12 8V7m0 1v8m0 0v1m0-1c-1.11 0-2.08-.402-2.599-1M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
      </svg>
    ),
  },
  {
    title: 'Live Progress Tracking',
    description: 'Real-time updates, screenshots, and direct booster communication.',
    icon: (
      <svg className="w-8 h-8" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="2">
        <path strokeLinecap="round" strokeLinejoin="round" d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z" />
      </svg>
    ),
  },
  {
    title: 'Account Security',
    description: 'Encrypted credential storage, no third-party access, secure handoff.',
    icon: (
      <svg className="w-8 h-8" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="2">
        <path strokeLinecap="round" strokeLinejoin="round" d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
      </svg>
    ),
  },
];

const ranks = [
  { name: 'Silver I - IV', wingman: '$15', premier: '$25' },
  { name: 'Gold Nova I - III', wingman: '$25', premier: '$40' },
  { name: 'Gold Nova Master', wingman: '$35', premier: '$55' },
  { name: 'Master Guardian I - II', wingman: '$45', premier: '$70' },
  { name: 'Master Guardian Elite', wingman: '$55', premier: '$90' },
  { name: 'Distinguished Master Guardian', wingman: '$70', premier: '$110' },
  { name: 'Legendary Eagle', wingman: '$90', premier: '$140' },
  { name: 'Legendary Eagle Master', wingman: '$110', premier: '$180' },
  { name: 'Supreme Master First Class', wingman: '$140', premier: '$230' },
  { name: 'Global Elite', wingman: '$180', premier: '$300' },
];

export default function HomePage() {
  return (
    <div className="min-h-screen">
      <header className="border-b border-[var(--border)] sticky top-0 z-40 bg-[var(--background)]/95 backdrop-blur supports-[backdrop-filter]:bg-[var(--background)]/80">
        <div className="container">
          <div className="flex h-16 items-center justify-between">
            <Link href="/" className="font-bold text-xl tracking-tight" aria-label="CS2 Boost Home">
              CS2 Boost
            </Link>
            <nav className="flex items-center gap-6">
              <Link href="#features" className="text-sm font-medium text-[var(--muted-foreground)] hover:text-[var(--foreground)] transition-colors">
                Features
              </Link>
              <Link href="#pricing" className="text-sm font-medium text-[var(--muted-foreground)] hover:text-[var(--foreground)] transition-colors">
                Pricing
              </Link>
              <Link href="/rushcup" className="btn btn-primary text-sm">
                RUSH CUP
              </Link>
            </nav>
          </div>
        </div>
      </header>

      <main>
        <section className="relative py-20 lg:py-32 overflow-hidden">
          <div className="container">
            <div className="max-w-3xl mx-auto text-center">
              <h1 className="text-4xl lg:text-6xl font-bold tracking-tight mb-6">
                Professional CS2
                <br />
                <span className="text-[var(--primary)]">Boosting Services</span>
              </h1>
              <p className="text-lg lg:text-xl text-[var(--muted-foreground)] mb-8 max-w-2xl mx-auto">
                Reach Global Elite in Wingman or 20k+ Premier rating. Verified boosters, crypto payments, live tracking.
              </p>
              <div className="flex flex-col sm:flex-row items-center justify-center gap-4">
                <a href="mailto:boost@cs2boost.com?subject=Boost%20Inquiry" className="btn btn-primary text-lg px-8 py-3">
                  Contact Us
                </a>
                <Link href="#pricing" className="btn btn-outline text-lg px-8 py-3">
                  View Pricing
                </Link>
              </div>
            </div>
          </div>
        </section>

        <section id="features" className="py-20 lg:py-28 bg-[var(--muted)]/50">
          <div className="container">
            <div className="text-center mb-16">
              <h2 className="text-3xl lg:text-4xl font-bold mb-4">Why Choose Us</h2>
              <p className="text-[var(--muted-foreground)] max-w-2xl mx-auto">
                Built for players who demand quality, security, and results.
              </p>
            </div>
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
              {features.map((feature, index) => (
                <article key={index} className="card group hover:border-[var(--primary)]/50 transition-colors">
                  <div className="text-[var(--primary)] mb-4">{feature.icon}</div>
                  <h3 className="text-lg font-semibold mb-2">{feature.title}</h3>
                  <p className="text-[var(--muted-foreground)]">{feature.description}</p>
                </article>
              ))}
            </div>
          </div>
        </section>

        <section id="pricing" className="py-20 lg:py-28">
          <div className="container">
            <div className="text-center mb-16">
              <h2 className="text-3xl lg:text-4xl font-bold mb-4">Transparent Pricing</h2>
              <p className="text-[var(--muted-foreground)] max-w-2xl mx-auto">
                Per-rank pricing. No hidden fees. Pay only for what you need.
              </p>
            </div>
            <div className="overflow-x-auto">
              <table className="table w-full">
                <thead>
                  <tr>
                    <th className="w-1/3">Current Rank</th>
                    <th className="text-center">Wingman</th>
                    <th className="text-center">Premier</th>
                  </tr>
                </thead>
                <tbody>
                  {ranks.map((rank, index) => (
                    <tr key={index}>
                      <td className="font-medium">{rank.name}</td>
                      <td className="text-center font-mono font-semibold">{rank.wingman}</td>
                      <td className="text-center font-mono font-semibold">{rank.premier}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
            <p className="text-center text-[var(--muted-foreground)] mt-6 text-sm">
              Prices in USD equivalent. Crypto payments accepted (USDT, BTC, ETH). Custom quotes for bulk orders.
            </p>
          </div>
        </section>

        <section className="py-20 lg:py-28 bg-[var(--muted)]/50">
          <div className="container">
            <div className="max-w-3xl mx-auto text-center">
              <h2 className="text-3xl lg:text-4xl font-bold mb-6">Ready to Rank Up?</h2>
              <p className="text-[var(--muted-foreground)] mb-8">
                Contact us for a custom quote and get matched with a verified booster.
              </p>
              <a href="mailto:boost@cs2boost.com?subject=Boost%20Inquiry" className="btn btn-primary text-lg px-8 py-3 inline-flex">
                Get a Quote
              </a>
            </div>
          </div>
        </section>
      </main>

      <footer className="border-t border-[var(--border)] py-12 bg-[var(--muted)]/30">
        <div className="container">
          <div className="grid grid-cols-1 md:grid-cols-4 gap-8">
            <div>
              <h3 className="font-bold mb-4">CS2 Boost</h3>
              <p className="text-[var(--muted-foreground)] text-sm">
                Professional Counter-Strike 2 boosting services. Secure, fast, reliable.
              </p>
            </div>
            <div>
              <h4 className="font-semibold mb-4">Services</h4>
              <ul className="space-y-2 text-sm text-[var(--muted-foreground)]">
                <li><Link href="#pricing" className="hover:text-[var(--foreground)]">Wingman Boosting</Link></li>
                <li><Link href="#pricing" className="hover:text-[var(--foreground)]">Premier Boosting</Link></li>
                <li><Link href="#pricing" className="hover:text-[var(--foreground)]">Competitive Boosting</Link></li>
                <li><Link href="#pricing" className="hover:text-[var(--foreground)]">Faceit Boosting</Link></li>
              </ul>
            </div>
            <div>
              <h4 className="font-semibold mb-4">Support</h4>
              <ul className="space-y-2 text-sm text-[var(--muted-foreground)]">
                <li><Link href="/terms" className="hover:text-[var(--foreground)]">Terms of Service</Link></li>
                <li><Link href="/privacy" className="hover:text-[var(--foreground)]">Privacy Policy</Link></li>
                <li><Link href="/rushcup" className="hover:text-[var(--foreground)]">RUSH CUP Tournament</Link></li>
              </ul>
            </div>
            <div>
              <h4 className="font-semibold mb-4">Payment</h4>
              <ul className="space-y-2 text-sm text-[var(--muted-foreground)]">
                <li>USDT (TRC20, ERC20)</li>
                <li>Bitcoin (BTC)</li>
                <li>Ethereum (ETH)</li>
                <li>Litecoin (LTC)</li>
              </ul>
            </div>
          </div>
          <div className="border-t border-[var(--border)] mt-8 pt-8 text-center text-sm text-[var(--muted-foreground)]">
            <p>© {new Date().getFullYear()} CS2 Boost. All rights reserved.</p>
          </div>
        </div>
      </footer>
    </div>
  );
}