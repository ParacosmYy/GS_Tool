#include "g32566/m32566.h"
QVector<double> m32566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
