#include "k15830/m15830.h"
QVector<double> m15830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
