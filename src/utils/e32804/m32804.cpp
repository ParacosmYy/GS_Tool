#include "e32804/m32804.h"
QVector<double> m32804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
