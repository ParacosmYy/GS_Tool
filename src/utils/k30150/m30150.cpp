#include "k30150/m30150.h"
QVector<double> m30150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
