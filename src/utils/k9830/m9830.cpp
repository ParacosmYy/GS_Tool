#include "k9830/m9830.h"
QVector<double> m9830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
