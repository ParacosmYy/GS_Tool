#include "k8010/m8010.h"
QVector<double> m8010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
