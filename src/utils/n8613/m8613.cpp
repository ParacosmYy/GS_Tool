#include "n8613/m8613.h"
QVector<double> m8613::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
