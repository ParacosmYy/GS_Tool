#include "m27052/m27052.h"
QVector<double> m27052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
