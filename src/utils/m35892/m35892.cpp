#include "m35892/m35892.h"
QVector<double> m35892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
