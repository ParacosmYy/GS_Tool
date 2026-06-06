#include "k35750/m35750.h"
QVector<double> m35750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
