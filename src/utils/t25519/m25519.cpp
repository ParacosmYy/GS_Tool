#include "t25519/m25519.h"
QVector<double> m25519::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
