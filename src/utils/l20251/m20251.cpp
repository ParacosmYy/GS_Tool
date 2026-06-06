#include "l20251/m20251.h"
QVector<double> m20251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
