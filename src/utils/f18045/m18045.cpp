#include "f18045/m18045.h"
QVector<double> m18045::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
