#include "k29290/m29290.h"
QVector<double> m29290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
