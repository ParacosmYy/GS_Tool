#include "l35011/m35011.h"
QVector<double> m35011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
