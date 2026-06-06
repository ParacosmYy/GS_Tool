#include "f35685/m35685.h"
QVector<double> m35685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
