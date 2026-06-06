#include "k8510/m8510.h"
QVector<double> m8510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
