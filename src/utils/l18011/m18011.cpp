#include "l18011/m18011.h"
QVector<double> m18011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
