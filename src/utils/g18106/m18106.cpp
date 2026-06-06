#include "g18106/m18106.h"
QVector<double> m18106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
