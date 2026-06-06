#include "i18328/m18328.h"
QVector<double> m18328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
