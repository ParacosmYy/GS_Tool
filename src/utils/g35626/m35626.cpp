#include "g35626/m35626.h"
QVector<double> m35626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
