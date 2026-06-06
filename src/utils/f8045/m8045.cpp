#include "f8045/m8045.h"
QVector<double> m8045::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
