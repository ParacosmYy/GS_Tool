#include "m16412/m16412.h"
QVector<double> m16412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
