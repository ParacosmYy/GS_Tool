#include "a25720/m25720.h"
QVector<double> m25720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
