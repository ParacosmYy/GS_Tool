#include "f35125/m35125.h"
QVector<double> m35125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
