#include "a32420/m32420.h"
QVector<double> m32420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
