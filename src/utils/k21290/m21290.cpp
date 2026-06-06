#include "k21290/m21290.h"
QVector<double> m21290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
