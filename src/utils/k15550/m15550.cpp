#include "k15550/m15550.h"
QVector<double> m15550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
