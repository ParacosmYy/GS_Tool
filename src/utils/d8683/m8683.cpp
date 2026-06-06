#include "d8683/m8683.h"
QVector<double> m8683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
