#include "k8790/m8790.h"
QVector<double> m8790::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
