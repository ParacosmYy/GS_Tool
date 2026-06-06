#include "d35683/m35683.h"
QVector<double> m35683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
