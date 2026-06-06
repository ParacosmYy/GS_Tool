#include "i35488/m35488.h"
QVector<double> m35488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
