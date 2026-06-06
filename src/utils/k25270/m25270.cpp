#include "k25270/m25270.h"
QVector<double> m25270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
