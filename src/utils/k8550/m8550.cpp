#include "k8550/m8550.h"
QVector<double> m8550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
