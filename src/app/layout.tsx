import type { Metadata, Viewport } from 'next';
import { Inter, JetBrains_Mono } from 'next/font/google';
import './globals.css';

const inter = Inter({
  subsets: ['latin'],
  display: 'swap',
  variable: '--font-inter',
});

const jetbrainsMono = JetBrains_Mono({
  subsets: ['latin'],
  display: 'swap',
  variable: '--font-jetbrains',
});

export const metadata: Metadata = {
  title: 'CS2 Boost - Professional Ranking Service',
  description: 'Professional CS2 boosting services. Global Elite Wingman, 20k+ Premier. Secure, fast, reliable.',
  keywords: ['CS2', 'Counter-Strike 2', 'boosting', 'ranking', 'Global Elite', 'Premier', 'Wingman'],
  authors: [{ name: 'CS2 Boost' }],
  creator: 'CS2 Boost',
  publisher: 'CS2 Boost',
  robots: 'index, follow',
  openGraph: {
    type: 'website',
    locale: 'en_US',
    url: 'https://cs2boost.com',
    siteName: 'CS2 Boost',
    title: 'CS2 Boost - Professional Ranking Service',
    description: 'Professional CS2 boosting services. Global Elite Wingman, 20k+ Premier.',
    images: [
      {
        url: '/og-image.png',
        width: 1200,
        height: 630,
        alt: 'CS2 Boost',
      },
    ],
  },
  twitter: {
    card: 'summary_large_image',
    title: 'CS2 Boost - Professional Ranking Service',
    description: 'Professional CS2 boosting services. Global Elite Wingman, 20k+ Premier.',
    images: ['/og-image.png'],
  },
  icons: {
    icon: '/favicon.svg',
    shortcut: '/favicon.svg',
    apple: '/apple-touch-icon.png',
  },
  manifest: '/site.webmanifest',
};

export const viewport: Viewport = {
  themeColor: [
    { media: '(prefers-color-scheme: light)', color: '#ffffff' },
    { media: '(prefers-color-scheme: dark)', color: '#0a0a0b' },
  ],
  width: 'device-width',
  initialScale: 1,
  maximumScale: 5,
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en" className={`${inter.variable} ${jetbrainsMono.variable}`} suppressHydrationWarning>
      <head>
        <link rel="preconnect" href="https://fonts.googleapis.com" />
        <link rel="preconnect" href="https://fonts.gstatic.com" crossOrigin="anonymous" />
      </head>
      <body className="min-h-screen bg-[var(--background)] text-[var(--foreground)] antialiased">
        {children}
      </body>
    </html>
  );
}