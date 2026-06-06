#include "p15575/m15575.h"
QVector<double> m15575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
