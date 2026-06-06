#include "p24575/m24575.h"
QVector<double> m24575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
