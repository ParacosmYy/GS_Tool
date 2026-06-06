#include "k35710/m35710.h"
QVector<double> m35710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
