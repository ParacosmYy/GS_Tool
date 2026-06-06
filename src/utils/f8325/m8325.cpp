#include "f8325/m8325.h"
QVector<double> m8325::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
