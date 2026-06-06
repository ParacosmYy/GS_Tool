#include "k8090/m8090.h"
QVector<double> m8090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
