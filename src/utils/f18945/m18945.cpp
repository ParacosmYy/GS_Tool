#include "f18945/m18945.h"
QVector<double> m18945::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
