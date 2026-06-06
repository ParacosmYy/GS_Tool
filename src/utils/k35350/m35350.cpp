#include "k35350/m35350.h"
QVector<double> m35350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
