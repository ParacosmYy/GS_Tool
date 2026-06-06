#include "k9750/m9750.h"
QVector<double> m9750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
