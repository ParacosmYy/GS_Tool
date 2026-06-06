#include "k8190/m8190.h"
QVector<double> m8190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
