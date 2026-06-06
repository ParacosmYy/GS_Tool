#include "k8890/m8890.h"
QVector<double> m8890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
