#include "i26508/m26508.h"
QVector<double> m26508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
